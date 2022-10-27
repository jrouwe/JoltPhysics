// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "Layers.h"

TEST_SUITE("WheeledVehicleTests")
{
	// Simplified vehicle settings
	struct VehicleSettings
	{
		Vec3		mPosition { 0, 2, 0 };
		bool		mUseCastSphere = true;
		float		mWheelRadius = 0.3f;
		float		mWheelWidth = 0.1f;
		float		mHalfVehicleLength = 2.0f;
		float		mHalfVehicleWidth = 0.9f;
		float		mHalfVehicleHeight = 0.2f;
		float		mWheelOffsetHorizontal = 1.4f;
		float		mWheelOffsetVertical = 0.18f;
		float		mSuspensionMinLength = 0.3f;
		float		mSuspensionMaxLength = 0.5f;
		float		mMaxSteeringAngle = DegreesToRadians(30);
		bool		mFourWheelDrive = false;
		bool		mAntiRollbar = true;
	};

	// Helper function to create a vehicle
	static VehicleConstraint *AddVehicle(PhysicsTestContext &inContext, VehicleSettings &inSettings)
	{
		// Create vehicle body
		RefConst<Shape> car_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -inSettings.mHalfVehicleHeight, 0), new BoxShape(Vec3(inSettings.mHalfVehicleWidth, inSettings.mHalfVehicleHeight, inSettings.mHalfVehicleLength))).Create().Get();
		BodyCreationSettings car_body_settings(car_shape, inSettings.mPosition, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
		car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;
		Body *car_body = inContext.GetBodyInterface().CreateBody(car_body_settings);
		inContext.GetBodyInterface().AddBody(car_body->GetID(), EActivation::Activate);

		// Create vehicle constraint
		VehicleConstraintSettings vehicle;
		vehicle.mDrawConstraintSize = 0.1f;
		vehicle.mMaxPitchRollAngle = DegreesToRadians(60.0f);

		// Wheels
		WheelSettingsWV *w1 = new WheelSettingsWV;
		w1->mPosition = Vec3(inSettings.mHalfVehicleWidth, -inSettings.mWheelOffsetVertical, inSettings.mWheelOffsetHorizontal);
		w1->mMaxSteerAngle = inSettings.mMaxSteeringAngle;
		w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

		WheelSettingsWV *w2 = new WheelSettingsWV;
		w2->mPosition = Vec3(-inSettings.mHalfVehicleWidth, -inSettings.mWheelOffsetVertical, inSettings.mWheelOffsetHorizontal);
		w2->mMaxSteerAngle = inSettings.mMaxSteeringAngle;
		w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

		WheelSettingsWV *w3 = new WheelSettingsWV;
		w3->mPosition = Vec3(inSettings.mHalfVehicleWidth, -inSettings.mWheelOffsetVertical, -inSettings.mWheelOffsetHorizontal);
		w3->mMaxSteerAngle = 0.0f;

		WheelSettingsWV *w4 = new WheelSettingsWV;
		w4->mPosition = Vec3(-inSettings.mHalfVehicleWidth, -inSettings.mWheelOffsetVertical, -inSettings.mWheelOffsetHorizontal);
		w4->mMaxSteerAngle = 0.0f;

		vehicle.mWheels = { w1, w2, w3, w4 };
	
		for (WheelSettings *w : vehicle.mWheels)
		{
			w->mRadius = inSettings.mWheelRadius;
			w->mWidth = inSettings.mWheelWidth;
			w->mSuspensionMinLength = inSettings.mSuspensionMinLength;
			w->mSuspensionMaxLength = inSettings.mSuspensionMaxLength;
		}

		WheeledVehicleControllerSettings *controller = new WheeledVehicleControllerSettings;
		vehicle.mController = controller;

		// Differential
		controller->mDifferentials.resize(inSettings.mFourWheelDrive? 2 : 1);
		controller->mDifferentials[0].mLeftWheel = 0;
		controller->mDifferentials[0].mRightWheel = 1;
		if (inSettings.mFourWheelDrive)
		{
			controller->mDifferentials[1].mLeftWheel = 2;
			controller->mDifferentials[1].mRightWheel = 3;

			// Split engine torque
			controller->mDifferentials[0].mEngineTorqueRatio = controller->mDifferentials[1].mEngineTorqueRatio = 0.5f;
		}

		// Anti rollbars
		if (inSettings.mAntiRollbar)
		{
			vehicle.mAntiRollBars.resize(2);
			vehicle.mAntiRollBars[0].mLeftWheel = 0;
			vehicle.mAntiRollBars[0].mRightWheel = 1;
			vehicle.mAntiRollBars[1].mLeftWheel = 2;
			vehicle.mAntiRollBars[1].mRightWheel = 3;
		}

		// Create the constraint
		VehicleConstraint *constraint = new VehicleConstraint(*car_body, vehicle);

		// Create collision tester
		RefConst<VehicleCollisionTester> tester;
		if (inSettings.mUseCastSphere)
			tester = new VehicleCollisionTesterCastSphere(Layers::MOVING, 0.5f * inSettings.mWheelWidth);
		else
			tester = new VehicleCollisionTesterRay(Layers::MOVING);
		constraint->SetVehicleCollisionTester(tester);

		// Add to the world
		inContext.GetSystem()->AddConstraint(constraint);
		inContext.GetSystem()->AddStepListener(constraint);
		return constraint;
	}

	static void CheckOnGround(VehicleConstraint *inConstraint, const VehicleSettings &inSettings, const BodyID &inGroundID)
	{
		// Between min and max suspension length
		Vec3 pos = inConstraint->GetVehicleBody()->GetPosition();
		CHECK(pos.GetY() > inSettings.mSuspensionMinLength + inSettings.mWheelOffsetVertical + inSettings.mHalfVehicleHeight); 
		CHECK(pos.GetY() < inSettings.mSuspensionMaxLength + inSettings.mWheelOffsetVertical + inSettings.mHalfVehicleHeight);

		// Wheels touching ground
		for (const Wheel *w : inConstraint->GetWheels()) 
			CHECK(w->GetContactBodyID() == inGroundID);
	}

	TEST_CASE("TestBasicWheeledVehicle")
	{
		PhysicsTestContext c;
		BodyID floor_id = c.CreateFloor().GetID();

		VehicleSettings settings;
		VehicleConstraint *constraint = AddVehicle(c, settings);
		Body *body = constraint->GetVehicleBody();
		WheeledVehicleController *controller = static_cast<WheeledVehicleController *>(constraint->GetController());

		// Should start at specified position
		CHECK_APPROX_EQUAL(body->GetPosition(), settings.mPosition);

		// After 1 step we should not be at ground yet
		c.SimulateSingleStep();
		for (const Wheel *w : constraint->GetWheels())
			CHECK(w->GetContactBodyID().IsInvalid());
		CHECK(controller->GetTransmission().GetCurrentGear() == 0);

		// After 1 second we should be on ground but not moving horizontally
		c.Simulate(1.0f);
		CheckOnGround(constraint, settings, floor_id);
		Vec3 pos1 = body->GetPosition();
		CHECK_APPROX_EQUAL(pos1.GetX(), 0); // Not moving horizontally
		CHECK_APPROX_EQUAL(pos1.GetZ(), 0);
		CHECK(controller->GetTransmission().GetCurrentGear() == 0);

		// Start driving forward
		controller->SetDriverInput(1.0f, 0.0f, 0.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(1.0f);
		CheckOnGround(constraint, settings, floor_id);
		Vec3 pos2 = body->GetPosition();
		CHECK_APPROX_EQUAL(pos2.GetX(), 0, 1.0e-3f); // Not moving left/right
		CHECK(pos2.GetZ() > pos1.GetZ() + 1.0f); // Moving in Z direction
		Vec3 vel = body->GetLinearVelocity();
		CHECK_APPROX_EQUAL(vel.GetX(), 0, 1.0e-2f); // Not moving left/right
		CHECK(vel.GetZ() > 1.0f); // Moving in Z direction
		CHECK(controller->GetTransmission().GetCurrentGear() > 0);

		// Brake
		controller->SetDriverInput(0.0f, 0.0f, 1.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(2.0f);
		CheckOnGround(constraint, settings, floor_id);
		CHECK(!body->IsActive()); // Car should have gone sleeping
		Vec3 pos3 = body->GetPosition();
		CHECK_APPROX_EQUAL(pos3.GetX(), 0, 1.0e-3f); // Not moving left/right
		CHECK(pos3.GetZ() > pos2.GetZ() + 1.0f); // Moving in Z direction while braking
		vel = body->GetLinearVelocity();
		CHECK_APPROX_EQUAL(vel, Vec3::sZero(), 1.0e-3f); // Not moving

		// Start driving backwards
		controller->SetDriverInput(-1.0f, 0.0f, 0.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(2.0f);
		CheckOnGround(constraint, settings, floor_id);
		Vec3 pos4 = body->GetPosition();
		CHECK_APPROX_EQUAL(pos4.GetX(), 0, 1.0e-2f); // Not moving left/right
		CHECK(pos4.GetZ() < pos3.GetZ() - 1.0f); // Moving in -Z direction
		vel = body->GetLinearVelocity();
		CHECK_APPROX_EQUAL(vel.GetX(), 0, 1.0e-2f); // Not moving left/right
		CHECK(vel.GetZ() < -1.0f); // Moving in -Z direction
		CHECK(controller->GetTransmission().GetCurrentGear() < 0);

		// Brake
		controller->SetDriverInput(0.0f, 0.0f, 1.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(3.0f);
		CheckOnGround(constraint, settings, floor_id);
		CHECK(!body->IsActive()); // Car should have gone sleeping
		Vec3 pos5 = body->GetPosition();
		CHECK_APPROX_EQUAL(pos5.GetX(), 0, 1.0e-2f); // Not moving left/right
		CHECK(pos5.GetZ() < pos4.GetZ() - 1.0f); // Moving in -Z direction while braking
		vel = body->GetLinearVelocity();
		CHECK_APPROX_EQUAL(vel, Vec3::sZero(), 1.0e-3f); // Not moving

		// Turn right
		controller->SetDriverInput(1.0f, 1.0f, 0.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(1.0f);
		CheckOnGround(constraint, settings, floor_id);
		Vec3 omega = body->GetAngularVelocity();
		CHECK(omega.GetY() < -0.4f); // Rotating right
		CHECK(controller->GetTransmission().GetCurrentGear() > 0);

		// Hand brake
		controller->SetDriverInput(0.0f, 0.0f, 0.0f, 1.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(4.0f);
		CheckOnGround(constraint, settings, floor_id);
		CHECK(!body->IsActive()); // Car should have gone sleeping
		vel = body->GetLinearVelocity();
		CHECK_APPROX_EQUAL(vel, Vec3::sZero(), 1.0e-3f); // Not moving

		// Turn left
		controller->SetDriverInput(1.0f, -1.0f, 0.0f, 0.0f);
		c.GetBodyInterface().ActivateBody(body->GetID());
		c.Simulate(1.0f);
		CheckOnGround(constraint, settings, floor_id);
		omega = body->GetAngularVelocity();
		CHECK(omega.GetY() > 0.4f); // Rotating left
		CHECK(controller->GetTransmission().GetCurrentGear() > 0);
	}
}
