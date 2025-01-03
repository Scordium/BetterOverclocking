#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "ModConfig_BetterOverclockingStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/BetterOverclocking/ModConfig_BetterOverclocking' */
USTRUCT(BlueprintType)
struct FModConfig_BetterOverclockingStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float OverclockModifier{};

    UPROPERTY(BlueprintReadWrite)
    float OverclockExponent{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FModConfig_BetterOverclockingStruct GetActiveConfig(UObject* WorldContext) {
        FModConfig_BetterOverclockingStruct ConfigStruct{};
        FConfigId ConfigId{"BetterOverclocking", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FModConfig_BetterOverclockingStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

