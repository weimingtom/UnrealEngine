// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UMGPrivatePCH.h"

#include "UMGSequencePlayer.h"
#include "SceneViewport.h"

TSharedRef<FUMGDragDropOp> FUMGDragDropOp::New()
{
	TSharedRef<FUMGDragDropOp> Operation = MakeShareable(new FUMGDragDropOp);
	Operation->Construct();

	return Operation;
}

void FUMGDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
}

void FUMGDragDropOp::OnDragged(const class FDragDropEvent& DragDropEvent)
{
	FDragDropOperation::OnDragged(DragDropEvent);
}

TSharedPtr<SWidget> FUMGDragDropOp::GetDefaultDecorator() const
{
	return DecoratorWidget;
}

class SViewportWidgetHost : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SViewportWidgetHost)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

		SLATE_DEFAULT_SLOT(FArguments, Content)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, bool bInModal)
	{
		bModal = bInModal;

		ChildSlot
			[
				InArgs._Content.Widget
			];
	}

	//virtual bool OnHitTest(const FGeometry& MyGeometry, FVector2D InAbsoluteCursorPosition) override
	//{
	//	return false;
	//}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	FReply OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
	{
		// if we support focus, release the focus captures and lock the focus to this widget 
		if ( SupportsKeyboardFocus() )
		{
			return FReply::Handled().ReleaseMouseCapture().ReleaseJoystickCapture().LockMouseToWidget(SharedThis(this));
		}
		else
		{
			return FReply::Unhandled();
		}
	}

	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
	{
		// If we support focus, show the default mouse cursor 
		if ( SupportsKeyboardFocus() )
		{
			return FCursorReply::Cursor(EMouseCursor::Default);
		}
		else
		{
			return SCompoundWidget::OnCursorQuery(MyGeometry, CursorEvent);
		}
	}

	//FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent)
	//{
	//	return FReply::Handled();
	//}

	//FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent)
	//{
	//	return FReply::Handled();
	//}

	//FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	//{
	//	if ( bModal )
	//	{
	//		return FReply::Handled();
	//	}

	//	return FReply::Unhandled();
	//}

	//FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	//{
	//	if ( bModal )
	//	{
	//		return FReply::Handled();
	//	}

	//	return FReply::Unhandled();
	//}

	//FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	//{
	//	if ( bModal )
	//	{
	//		return FReply::Handled();
	//	}

	//	return FReply::Unhandled();
	//}

protected:
	bool bModal;
};


/////////////////////////////////////////////////////
// UUserWidget

UUserWidget::UUserWidget(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	/*PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;*/
	bShowCursorWhenVisible = false;
	bAbsoluteLayout = false;
	bModal = false;
	AbsoluteSize = FVector2D(100, 100);
	HorizontalAlignment = HAlign_Fill;
	VerticalAlignment = VAlign_Fill;

	Visiblity = ESlateVisibility::SelfHitTestInvisible;
}

void UUserWidget::ReleaseNativeWidget()
{
	Super::ReleaseNativeWidget();

	UWidget* RootWidget = GetRootWidgetComponent();
	if ( RootWidget )
	{
		RootWidget->ReleaseNativeWidget();
	}
}

void UUserWidget::PostInitProperties()
{
	Super::PostInitProperties();

	Components.Reset();

	// Only do this if this widget is of a blueprint class
	UWidgetBlueprintGeneratedClass* BGClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
	if ( BGClass != NULL )
	{
		BGClass->InitializeWidget(this);
	}

	//TODO UMG For non-BP versions how do we generate the Components list?
}

UWorld* UUserWidget::GetWorld() const
{
	if ( PlayerContext.IsValid() )
	{
		return PlayerContext.GetWorld();
	}

	//if ( ULevel* Level = Cast<ULevel>(Outer) )
	//{
	//	return Level->OwningWorld;
	//}

	return NULL;
}

void UUserWidget::PlayAnimation(FName AnimationName)
{
	UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
	if (BPClass)
	{
		const FWidgetAnimation* Animation = BPClass->FindAnimation(AnimationName);
		if( Animation )
		{
			// @todo UMG sequencer - Restart animations which have had Play called on them?
			UUMGSequencePlayer** FoundPlayer = ActiveSequencePlayers.FindByPredicate( [&](const UUMGSequencePlayer* Player) { return Player->GetMovieScene() == Animation->MovieScene; } );

			if( !FoundPlayer )
			{
				UUMGSequencePlayer* NewPlayer = ConstructObject<UUMGSequencePlayer>( UUMGSequencePlayer::StaticClass(), this );
				ActiveSequencePlayers.Add( NewPlayer );

				NewPlayer->OnSequenceFinishedPlaying().AddUObject( this, &UUserWidget::OnAnimationFinishedPlaying );

				NewPlayer->InitSequencePlayer( *Animation, *this );

				NewPlayer->Play();
			}
			else
			{
				(*FoundPlayer)->Play();
			}
		}
	}
}

void UUserWidget::StopAnimation(FName AnimationName)
{
	UWidgetBlueprintGeneratedClass* BPClass = Cast<UWidgetBlueprintGeneratedClass>(GetClass());
	if(BPClass)
	{
		const FWidgetAnimation* Animation = BPClass->FindAnimation(AnimationName);
		if(Animation)
		{
			// @todo UMG sequencer - Restart animations which have had Play called on them?
			UUMGSequencePlayer** FoundPlayer = ActiveSequencePlayers.FindByPredicate([&](const UUMGSequencePlayer* Player) { return Player->GetMovieScene() == Animation->MovieScene; });
			if(FoundPlayer)
			{
				(*FoundPlayer)->Stop();
			}
		}
	}
}

void UUserWidget::OnAnimationFinishedPlaying( UUMGSequencePlayer& Player )
{
	ActiveSequencePlayers.Remove( &Player );
}

UWidget* UUserWidget::GetWidgetHandle(TSharedRef<SWidget> InWidget)
{
	TWeakObjectPtr<UWidget> VisualWidget = WidgetToComponent.FindRef(InWidget);
	return VisualWidget.Get();
}

TSharedRef<SWidget> UUserWidget::RebuildWidget()
{
	WidgetToComponent.Reset();
	
	TSharedPtr<SWidget> UserRootWidget;

	// Add the first component to the root of the widget surface.
	if ( Components.Num() > 0 )
	{
		UserRootWidget = Components[0]->TakeWidget();
	}
	else
	{
		UserRootWidget = SNew(SSpacer);
	}

	// Place all of our top-level children Slate wrapped components into the overlay
	for ( int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ++ComponentIndex )
	{
		UWidget* Handle = Components[ComponentIndex];
		TSharedPtr<SWidget> Widget = Handle->GetCachedWidget();
		if ( Widget.IsValid() )
		{
			WidgetToComponent.Add(Widget, Handle);
		}
	}

	if ( !IsDesignTime() )
	{
		// Notify the widget that it has been constructed.
		Construct();
	}

	return UserRootWidget.ToSharedRef();
}

TSharedPtr<SWidget> UUserWidget::GetWidgetFromName(const FString& Name) const
{
	for ( auto& Entry : WidgetToComponent )
	{
		if ( Entry.Value->GetName().Equals(Name, ESearchCase::IgnoreCase) )
		{
			return Entry.Key.Pin();
		}
	}

	return TSharedPtr<SWidget>();
}

UWidget* UUserWidget::GetHandleFromName(const FString& Name) const
{
	for ( UWidget* Widget : Components )
	{
		if ( Widget->GetName().Equals(Name, ESearchCase::IgnoreCase) )
		{
			return Widget;
		}
	}

	return NULL;
}

void UUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime )
{
	// Update active movie scenes
	for( UUMGSequencePlayer* Player : ActiveSequencePlayers )
	{
		Player->Tick( InDeltaTime );
	}

	Tick( MyGeometry, InDeltaTime );
}

TSharedRef<SWidget> UUserWidget::MakeFullScreenWidget()
{
	if ( bAbsoluteLayout )
	{
		return SNew(SCanvas)

			+ SCanvas::Slot()
			.Position(AbsolutePosition)
			.Size(AbsoluteSize)
			.VAlign(VerticalAlignment)
			.HAlign(HorizontalAlignment)
			[
				TakeWidget()
			];
	}
	else
	{
		TSharedRef<SVerticalBox> VerticalBox = SNew(SVerticalBox);

		auto& NewSlot = VerticalBox->AddSlot()
			.Padding(Padding)
			.HAlign(HorizontalAlignment)
			.VAlign(VerticalAlignment)
			[
				TakeWidget()
			];

		NewSlot.SizeParam = UWidget::ConvertSerializedSizeParamToRuntime(Size);

		return VerticalBox;
	}
}

UWidget* UUserWidget::GetRootWidgetComponent()
{
	if ( Components.Num() > 0 )
	{
		return Components[0];
	}

	return NULL;
}

void UUserWidget::Show()
{
	if ( !FullScreenWidget.IsValid() )
	{
		TSharedRef<SWidget> RootWidget = MakeFullScreenWidget();

		TSharedRef<SViewportWidgetHost> WidgetHost = SNew(SViewportWidgetHost, (bool)bModal)
			[
				RootWidget
			];

		FullScreenWidget = WidgetHost;

		//WidgetHost->SetVisibility(EVisibility::Visible);
		//OnVisibilityChanged.Broadcast(ESlateVisibility::Visible);

		// If this is a game world add the widget to the current worlds viewport.
		UWorld* World = GetWorld();
		if ( World && World->IsGameWorld() )
		{
			UGameViewportClient* Viewport = World->GetGameViewport();
			Viewport->AddViewportWidgetContent(WidgetHost);

			TWeakPtr<SViewport> GameViewportWidget = Viewport->GetGameViewport()->GetViewportWidget();
			if ( GameViewportWidget.IsValid() )
			{
				GameViewportWidget.Pin()->SetWidgetToFocusOnActivate(RootWidget);
				FSlateApplication::Get().SetKeyboardFocus(RootWidget);
			}
		}
	}
}

void UUserWidget::Hide()
{
	if ( FullScreenWidget.IsValid() )
	{
		TSharedPtr<SWidget> RootWidget = FullScreenWidget.Pin();

		//RootWidget->SetVisibility(EVisibility::Hidden);
		//OnVisibilityChanged.Broadcast(ESlateVisibility::Hidden);

		// If this is a game world add the widget to the current worlds viewport.
		UWorld* World = GetWorld();
		if ( World && World->IsGameWorld() )
		{
			UGameViewportClient* Viewport = World->GetGameViewport();
			Viewport->RemoveViewportWidgetContent(RootWidget.ToSharedRef());

			TWeakPtr<SViewport> GameViewportWidget = Viewport->GetGameViewport()->GetViewportWidget();
			if ( GameViewportWidget.IsValid() )
			{
				//TODO UMG this isn't what should manage focus, a higher level window controller, probably the viewport needs to understand
				// the Widget stack, and the dialog stack.
				GameViewportWidget.Pin()->ClearWidgetToFocusOnActivate();
				FSlateApplication::Get().SetKeyboardFocus(TSharedPtr<SWidget>());
			}
		}
	}
}

bool UUserWidget::GetIsVisible()
{
	return FullScreenWidget.IsValid();
}

TEnumAsByte<ESlateVisibility::Type> UUserWidget::GetVisiblity()
{
	if ( FullScreenWidget.IsValid() )
	{
		TSharedPtr<SWidget> RootWidget = FullScreenWidget.Pin();

		return UWidget::ConvertRuntimeToSerializedVisiblity(RootWidget->GetVisibility());
	}

	return ESlateVisibility::Collapsed;
}

void UUserWidget::SetPlayerContext(FLocalPlayerContext InPlayerContext)
{
	PlayerContext = InPlayerContext;
}

const FLocalPlayerContext& UUserWidget::GetPlayerContext() const
{
	return PlayerContext;
}

ULocalPlayer* UUserWidget::GetLocalPlayer() const
{
	APlayerController* PC = PlayerContext.IsValid() ? PlayerContext.GetPlayerController() : NULL;
	return PC ? Cast<ULocalPlayer>(PC->Player) : NULL;
}

APlayerController* UUserWidget::GetPlayerController() const
{
	return PlayerContext.IsValid() ? PlayerContext.GetPlayerController() : NULL;
}

#if WITH_EDITOR

const FSlateBrush* UUserWidget::GetEditorIcon()
{
	return FUMGStyle::Get().GetBrush("Widget.UserWidget");
}

#endif