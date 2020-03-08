// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkManagerDetails.h"

#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailCategoryBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#include "Input/Reply.h"
#include "VoxelWorld/ChunkManagement/ChunkManager.h"

#define LOCTEXT_NAMESPACE "ChunkManagerDetails"

TSharedRef<IDetailCustomization> FChunkManagerDetails::MakeInstance()
{
	return MakeShareable(new FChunkManagerDetails);
}



void FChunkManagerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsToEdit);


	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Settings|Voxel", LOCTEXT("CatName", "Voxel Settings"), ECategoryPriority::Important);


	Category.AddCustomRow(LOCTEXT("Keyword", "Shuffle"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NameText", "Reassign Materials"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("ButtonText", "Shuffle"))
			.OnClicked_Raw(this, &FChunkManagerDetails::EditObjects)
		];



}

FReply FChunkManagerDetails::EditObjects()
{
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid()) continue;

		AChunkManager* CM = Cast<AChunkManager>(Object.Get());
		if (!CM) continue;

		CM->ReshuffleAssetList();

	}




	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE