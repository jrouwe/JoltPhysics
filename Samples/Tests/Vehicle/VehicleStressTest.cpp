// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/VehicleStressTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleStressTest)
{
	JPH_ADD_BASE_CLASS(VehicleStressTest, VehicleTest)
}

VehicleStressTest::~VehicleStressTest()
{
	for (Ref<VehicleConstraint> &c : mVehicles)
		mPhysicsSystem->RemoveStepListener(c);
}

void VehicleStressTest::Initialize()
{
	CreateFloor();

	const float wheel_radius = 0.3f;
	const float wheel_width = 0.1f;
	const float half_vehicle_length = 2.0f;
	const float half_vehicle_width = 0.9f;
	const float half_vehicle_height = 0.2f;
	const float max_steering_angle = DegreesToRadians(30.0f);

	// Create vehicle body
	RefConst<Shape> car_shape = new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length));
	BodyCreationSettings car_body_settings(car_shape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	car_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	car_body_settings.mMassPropertiesOverride.mMass = 1500.0f;

	// Create vehicle constraint
	VehicleConstraintSettings vehicle;

	// Wheels, left front
	WheelSettingsWV *w1 = new WheelSettingsWV;
	w1->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w1->mMaxSteerAngle = max_steering_angle;
	w1->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	// Right front
	WheelSettingsWV *w2 = new WheelSettingsWV;
	w2->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, half_vehicle_length - 2.0f * wheel_radius);
	w2->mMaxSteerAngle = max_steering_angle;
	w2->mMaxHandBrakeTorque = 0.0f; // Front wheel doesn't have hand brake

	// Left rear
	WheelSettingsWV *w3 = new WheelSettingsWV;
	w3->mPosition = Vec3(half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w3->mMaxSteerAngle = 0.0f;

	// Right rear
	WheelSettingsWV *w4 = new WheelSettingsWV;
	w4->mPosition = Vec3(-half_vehicle_width, -0.9f * half_vehicle_height, -half_vehicle_length + 2.0f * wheel_radius);
	w4->mMaxSteerAngle = 0.0f;

	vehicle.mWheels = { w1, w2, w3, w4 };

	for (WheelSettings *w : vehicle.mWheels)
	{
		w->mRadius = wheel_radius;
		w->mWidth = wheel_width;
	}

	// Controller
	WheeledVehicleControllerSettings *controller = new WheeledVehicleControllerSettings;
	vehicle.mController = controller;

	// Differential
	controller->mDifferentials.resize(1);
	controller->mDifferentials[0].mLeftWheel = 0;
	controller->mDifferentials[0].mRightWheel = 1;

	for (int x = 0; x < 10; ++x)
		for (int y = 0; y < 10; ++y)
		{
			// Create body
			car_body_settings.mPosition = RVec3(-20.0f + x * 4.0f, 2.0f, -25.0f + y * 5.0f);
			Body *car_body = mBodyInterface->CreateBody(car_body_settings);
			mBodyInterface->AddBody(car_body->GetID(), EActivation::Activate);

			// Create constraint
			VehicleConstraint *c = new VehicleConstraint(*car_body, vehicle);
			c->SetNumStepsBetweenCollisionTestActive(2); // Only test collision every other step to speed up simulation
			c->SetNumStepsBetweenCollisionTestInactive(0); // Disable collision testing when inactive

			// Set the collision tester
			VehicleCollisionTester *tester = new VehicleCollisionTesterCastCylinder(Layers::MOVING);
			c->SetVehicleCollisionTester(tester);

			// Add the vehicle
			mPhysicsSystem->AddConstraint(c);
			mPhysicsSystem->AddStepListener(c);
			mVehicles.push_back(c);
		}
}

void VehicleStressTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Determine acceleration and brake
	float forward = 0.0f, right = 0.0f, hand_brake = 0.0f;
	if (inParams.mKeyboard->IsKeyPressed(DIK_UP))
		forward = 1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_DOWN))
		forward = -1.0f;

	// Steering
	if (inParams.mKeyboard->IsKeyPressed(DIK_LEFT))
		right = -1.0f;
	else if (inParams.mKeyboard->IsKeyPressed(DIK_RIGHT))
		right = 1.0f;

	// Hand brake will cancel gas pedal
	if (inParams.mKeyboard->IsKeyPressed(DIK_Z))
	{
		forward = 0.0f;
		hand_brake = 1.0f;
	}

	for (VehicleConstraint *c : mVehicles)
	{
		// On user input, assure that the car is active
		if (right != 0.0f || forward != 0.0f)
			mBodyInterface->ActivateBody(c->GetVehicleBody()->GetID());

		// Pass the input on to the constraint
		WheeledVehicleController *controller = static_cast<WheeledVehicleController *>(c->GetController());
		controller->SetDriverInput(forward, right, 0.0f, hand_brake);

		// Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
		for (uint w = 0; w < 4; ++w)
		{
			const WheelSettings *settings = c->GetWheels()[w]->GetSettings();
			RMat44 wheel_transform = c->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cyclinder we draw is aligned with Y so we specify that as rotational axis
			mDebugRenderer->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);
		}
	}
}
