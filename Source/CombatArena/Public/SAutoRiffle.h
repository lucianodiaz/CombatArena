#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SAutoRiffle.generated.h"


UCLASS()

class COMBATARENA_API ASAutoRiffle : public ASWeapon
{
	GENERATED_BODY()
public:
	ASAutoRiffle();

protected:
	virtual void BeginPlay() override;
	
	virtual void Fire() override;

private:
	virtual void shoot() override;
};
