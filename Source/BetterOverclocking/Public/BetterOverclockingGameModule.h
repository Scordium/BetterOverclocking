#pragma once

#include "FGInventoryComponent.h"
#include "FGPowerInfoComponent.h"
#include "ModConfig_BetterOverclockingStruct.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableManufacturerVariablePower.h"
#include "Module/GameWorldModule.h"

#include "BetterOverclockingGameModule.generated.h"


UCLASS()
class UBetterOverclockingGameModule : public UGameWorldModule
{
    GENERATED_BODY()

public:

    UBetterOverclockingGameModule() { bRootModule = true; }

    virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

    virtual void BeginDestroy() override
    {
        if(OverclockDelegate.IsValid()) OverclockDelegate.Reset();
        Super::BeginDestroy();
    }

    UFUNCTION(BlueprintCallable)
    void SetKValue() { KValue = FModConfig_BetterOverclockingStruct::GetActiveConfig(this).K_Value; }

    UFUNCTION(BlueprintCallable)
    void SetDValue() { DValue = FModConfig_BetterOverclockingStruct::GetActiveConfig(this).D_Value; }
    
    static inline float KValue = 1;
    static inline float DValue = 2;
    
    FDelegateHandle OverclockDelegate;

    static float CalculatePowerUsage(const AFGBuildableFactory* Object)
    {
        const auto ShardInventory = Object->GetPotentialInventory();
        
        const auto Shards = ShardInventory->GetNumItems(nullptr);
        const auto ClockSpeed = Object->GetCurrentPotential();
        float InitialPowerUsage = Object->mPowerConsumption;

        
        if(auto VariablePowerMachine = Cast<AFGBuildableManufacturerVariablePower>(Object))
        {
            auto Recipe = VariablePowerMachine->GetCurrentRecipe().GetDefaultObject();
            if(!Recipe) return VariablePowerMachine->GetIdlePowerConsumption();
			
            auto MinConsumption = Recipe->GetPowerConsumptionConstant();
            auto MaxConsumption = (Recipe->GetPowerConsumptionConstant() + Recipe->GetPowerConsumptionFactor());
            const auto PowerRange = FVector2D(MinConsumption, MaxConsumption);

            const auto CurrentCurvePos = VariablePowerMachine->mPowerConsumptionCurve->GetFloatValue(Object->GetProductionProgress());
            const auto CurrentUsage = FMath::GetMappedRangeValueClamped(FVector2D(0, 1), PowerRange, CurrentCurvePos);

            InitialPowerUsage = CurrentUsage;
        }

        auto _1 = KValue * InitialPowerUsage;
        auto _2 = (1.f - Shards/3.f) * FMath::Pow(ClockSpeed, DValue);
        auto _3 = InitialPowerUsage * KValue * ClockSpeed;

        auto OutputValue = FMath::Max(0.1f, _1 * _2 + _3);

        //Update power usage (required on load since machines don't update their consumption unless overclock was changed)
        auto PowerConnector = Object->GetPowerInfo();
        if(PowerConnector && PowerConnector->GetTargetConsumption() != OutputValue)
        {
            PowerConnector->SetTargetConsumption(OutputValue);
            PowerConnector->SetMaximumTargetConsumption(OutputValue);
        }

        return OutputValue;
    }
};
