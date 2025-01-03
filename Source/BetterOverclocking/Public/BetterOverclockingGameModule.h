#pragma once

#include "FGInventoryComponent.h"
#include "FGPowerInfoComponent.h"
#include "FGPowerShardDescriptor.h"
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

    virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

    virtual void BeginDestroy() override
    {
        if(OverclockDelegate.IsValid()) OverclockDelegate.Reset();
        Super::BeginDestroy();
    }

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
    TSubclassOf<UFGPowerShardDescriptor> PowerShardDescriptor;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
    TSubclassOf<UFGPowerShardDescriptor> SomersloopDescriptor;

    UFUNCTION(BlueprintCallable)
    void SetOverclockModifierValue() { OverclockModifier = FModConfig_BetterOverclockingStruct::GetActiveConfig(this).OverclockModifier; }

    UFUNCTION(BlueprintCallable)
    void SetOverclockExponentValue() { OverclockExponent = FModConfig_BetterOverclockingStruct::GetActiveConfig(this).OverclockExponent; }
    
    static inline float OverclockModifier = 1;
    static inline float OverclockExponent = 2;
    
    FDelegateHandle OverclockDelegate;

    float CalculatePowerUsage(const AFGBuildableFactory* Object)
    {
        const auto ShardInventory = Object->GetPotentialInventory();
        
        const auto Shards = ShardInventory->GetNumItems(PowerShardDescriptor);
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
        
        float SomersloopPowerMultiplier = FMath::Pow(Object->GetCurrentProductionBoost(), 2);

        auto _1 = OverclockModifier * SomersloopPowerMultiplier * InitialPowerUsage;
        auto _2 = (1.f - Shards/3.f) * FMath::Pow(ClockSpeed, OverclockExponent) + ClockSpeed;

        auto OutputValue = FMath::Max(0.1f, _1 * _2);

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
