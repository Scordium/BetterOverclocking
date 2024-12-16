#include "BetterOverclockingGameModule.h"

#include "ModConfig_BetterOverclockingStruct.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableGenerator.h"
#include "Buildables/FGBuildableManufacturerVariablePower.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Patching/NativeHookManager.h"

void UBetterOverclockingGameModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
    Super::DispatchLifecycleEvent(Phase);
    if(Phase != ELifecyclePhase::POST_INITIALIZATION) return;

    UKismetSystemLibrary::K2_SetTimer(this, "SetKValue", 1.0, true);
    UKismetSystemLibrary::K2_SetTimer(this, "SetDValue", 1.0, true);
    
    if(!WITH_EDITOR)
    {
        OverclockDelegate = SUBSCRIBE_METHOD(AFGBuildableFactory::GetProducingPowerConsumption, [](auto& Scope, const AFGBuildableFactory* Self) 
        {
            // If object is a generator, stop the execution and allow vanilla function to calculate the output.
            if(Self->IsA<AFGBuildableGenerator>()) return;
            if(!Self->GetPotentialInventory()) return;
        
            Scope.Override(CalculatePowerUsage(Self));
        });
        
        SUBSCRIBE_METHOD_VIRTUAL(AFGBuildableFactory::GetCurrentMaxPotential, AFGBuildableFactory::StaticClass()->ClassDefaultObject, [](auto& Scope, const AFGBuildableFactory* Self)
        {
            if(!Self->IsA<AFGBuildableGenerator>()) Scope.Override(2.5);
        });
    }
}

