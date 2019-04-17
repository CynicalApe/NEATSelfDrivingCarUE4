// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MLCharacter.generated.h"

UCLASS()
class MLTESTPROJECT_API AMLCharacter : public ACharacter
{
    GENERATED_BODY()

  public:
    // Sets default values for this character's properties
    AMLCharacter();

  protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

  public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    /** Called after an actor has been moved in the editor */
    virtual void PostEditMove(bool bFinished) override;
    virtual void PostLoad() override;
    virtual void PostActorCreated() override;
    // Handles input for moving forward and backward.
    UFUNCTION()
    void MoveForward(float Value);

    // Handles input for moving right and left.
    UFUNCTION()
    void MoveRight(float Value);

    // Sets jump flag when key is pressed.
    UFUNCTION()
    void StartJump();

    // Clears jump flag when key is released.
    UFUNCTION()
    void StopJump();

    UFUNCTION()
    void UpdateComponentLocations();

    UFUNCTION()
    void HandleAttachments();

    UFUNCTION()
    void UpdateEditorProperties();

    void HandleSingleAttachment(USceneComponent* ParentComponent,
                                USceneComponent* ComponentToAttach,
                                FName& SocketName);
    void TickSensors();
    UPROPERTY(EditAnywhere) class UStaticMeshComponent* static_mesh = nullptr;

    UPROPERTY(EditAnywhere)
    class USpringArmComponent* camera_spring_arm = nullptr;

    UPROPERTY(EditAnywhere)
    class UCameraComponent* attached_camera = nullptr;

    UPROPERTY(EditAnywhere)
    class USensorComponent* FrontSensor = nullptr;
    class USensorComponent* BehindSensor = nullptr;
    class USensorComponent* RightSensor = nullptr;
    class USensorComponent* LeftSensor = nullptr;

    // Input variables
    FVector2D MovementInput;
    FVector2D CameraInput;
    float ZoomFactor = 10.0f;
    bool bZoomingIn;

    // Input functions
    void PitchCamera(float AxisValue);
    void YawCamera(float AxisValue);
    void CameraZoom(float AxisValue);

    FName FrontSensorSocket;
    FName BehindSensorSocket;
    FName RightSensorSocket;
    FName LeftSensorSocket;

  private:
};
