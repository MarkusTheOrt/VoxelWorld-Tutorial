
#include "../Public/VoxelWorldEditor.h"

#include "VoxelWorld/ChunkManagement/ChunkManager.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "ChunkManagerDetails.h"

void FVoxelWorldEditorModule::StartupModule()
{
	UE_LOG(LogTemp, Warning, TEXT("VoxelWorld Editor Loaded."));

	FPropertyEditorModule& PropertyEdModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEdModule.RegisterCustomClassLayout(
		AChunkManager::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FChunkManagerDetails::MakeInstance)
	);
}

void FVoxelWorldEditorModule::ShutdownModule()
{
	UE_LOG(LogTemp, Warning, TEXT("VoxelWorld Editor shut down."));
}

IMPLEMENT_MODULE(FVoxelWorldEditorModule, VoxelWorldEditor);