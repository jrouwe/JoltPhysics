// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Helper class that either contains a valid result or an error
template <class Type>
class Result
{
public:
	/// Default constructor
						Result()									{ }
						
	/// Copy constructor
						Result(const Result<Type> &inRHS) :
		mState(inRHS.mState)
	{
		switch (inRHS.mState)
		{
		case EState::Valid:
			new (&mResult) Type (inRHS.mResult);
			break;

		case EState::Error:
			new (&mError) string(inRHS.mError);
			break;

		case EState::Invalid:
			break;
		}
	}

	/// Move constructor
						Result(Result<Type> &&inRHS) noexcept :
		mState(inRHS.mState)
	{
		switch (inRHS.mState)
		{
		case EState::Valid:
			new (&mResult) Type (move(inRHS.mResult));
			break;

		case EState::Error:
			new (&mError) string(move(inRHS.mError));
			break;

		case EState::Invalid:
			break;
		}

		inRHS.mState = EState::Invalid;
	}

	/// Destructor
						~Result()									{ Clear(); }

	/// Copy assignment
	Result<Type> &		operator = (const Result<Type> &inRHS)
	{
		Clear();

		mState = inRHS.mState;

		switch (inRHS.mState)
		{
		case EState::Valid:
			new (&mResult) Type (inRHS.mResult);
			break;

		case EState::Error:
			new (&mError) string(inRHS.mError);
			break;

		case EState::Invalid:
			break;
		}

		return *this;
	}

	/// Move assignment
	Result<Type> &		operator = (Result<Type> &&inRHS) noexcept
	{
		Clear();

		mState = inRHS.mState;

		switch (inRHS.mState)
		{
		case EState::Valid:
			new (&mResult) Type (move(inRHS.mResult));
			break;

		case EState::Error:
			new (&mError) string(move(inRHS.mError));
			break;

		case EState::Invalid:
			break;
		}

		inRHS.mState = EState::Invalid;

		return *this;
	}

	/// Clear result or error
	void				Clear()
	{ 
		switch (mState) 
		{ 
		case EState::Valid: 
			mResult.~Type(); 
			break; 

		case EState::Error:
			mError.~string();
			break;

		case EState::Invalid:
			break;
		}

		mState = EState::Invalid;
	}

	/// Checks if the result is still uninitialized
	bool				IsEmpty() const								{ return mState == EState::Invalid; }

	/// Checks if the result is valid
	bool				IsValid() const								{ return mState == EState::Valid; }

	/// Get the result value
	const Type &		Get() const									{ JPH_ASSERT(IsValid()); return mResult; }

	/// Set the result value
	void				Set(const Type &inResult)					{ Clear(); new (&mResult) Type(inResult); mState = EState::Valid; }

	/// Set the result value (move value)
	void				Set(const Type &&inResult)					{ Clear(); new (&mResult) Type(move(inResult)); mState = EState::Valid; }

	/// Check if we had an error
	bool				HasError() const							{ return mState == EState::Error; }

	/// Get the error value
	const string &		GetError() const							{ JPH_ASSERT(HasError()); return mError; }

	/// Set an error value
	void				SetError(const char *inError)				{ Clear(); new (&mError) string(inError); mState = EState::Error; }
	void				SetError(const string_view &inError)		{ Clear(); new (&mError) string(inError); mState = EState::Error; }
	void				SetError(string &&inError)					{ Clear(); new (&mError) string(move(inError)); mState = EState::Error; }

private:
	union
	{
		Type			mResult;									///< The actual result object
		string			mError;										///< The error description if the result failed
	};

	/// State of the result
	enum class EState : uint8
	{
		Invalid,
		Valid,
		Error
	};

	EState				mState = EState::Invalid;
};

JPH_NAMESPACE_END
