// Copyright Epic Games, Inc. All Rights Reserved.

#include "BetterOverclocking.h"

#define LOCTEXT_NAMESPACE "FBetterOverclockingModule"

void FBetterOverclockingModule::StartupModule()
{
}

void FBetterOverclockingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBetterOverclockingModule, BetterOverclocking)