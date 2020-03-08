
#include "CoreUObject.h"
#include "Modules/ModuleManager.h"

class FVoxelWorldEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

};