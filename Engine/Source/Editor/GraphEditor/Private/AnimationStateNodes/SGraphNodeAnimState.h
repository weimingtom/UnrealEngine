// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

//
// Forward declarations.
//
class UAnimStateNodeBase;
class UAnimStateConduitNode;
class UAnimStateEntryNode;

class SGraphNodeAnimState : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeAnimState){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UAnimStateNodeBase* InNode);

	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface

	// SGraphNode interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
	// End of SGraphNode interface

	static void GetStateInfoPopup(UEdGraphNode* GraphNode, TArray<FGraphInformationPopupInfo>& Popups);
protected:
	FSlateColor GetBorderBackgroundColor() const;

	virtual FText GetPreviewCornerText() const;
	virtual const FSlateBrush* GetNameIcon() const;
};


class SGraphNodeAnimConduit : public SGraphNodeAnimState
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeAnimConduit){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UAnimStateConduitNode* InNode);

	// SNodePanel::SNode interface
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override;
	// End of SNodePanel::SNode interface
protected:
	virtual FText GetPreviewCornerText() const override;
	virtual const FSlateBrush* GetNameIcon() const override;
};
