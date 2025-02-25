// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void LoadContentConfig()
{
	InitializeContentSystem();

	ContentSystem* contentSystem = PL::gContentSystem;

	Cog* configCog = PL::gEngine->GetConfigCog();
	MainConfig* mainConfig = configCog->has(MainConfig);

	Array<String>& librarySearchPaths = contentSystem->LibrarySearchPaths;
	ContentConfig* contentConfig = configCog->has(ContentConfig);

	String sourceDirectory = mainConfig->SourceDirectory;
	ErrorIf(sourceDirectory.Empty(), "Expected a source directory");

	String contentOutputDirectory = GetUserApplicationDirectory();

	if (contentConfig)
	{
		librarySearchPaths.InsertAt(0, contentConfig->LibraryDirectories.All());

		PlasmaPrint("Config has ContentOutput directory: '%s'\n", contentConfig->ContentOutput.c_str());

		if (contentConfig->ContentOutput.Empty() == false && DirectoryExists(contentConfig->ContentOutput))
		{
			contentOutputDirectory = contentConfig->ContentOutput;
		}
	}
	else
	{
		Warn("No ContentConfig found on Config Cog");
	}

	librarySearchPaths.PushBack(FilePath::Combine(sourceDirectory, "Resources"));

	contentSystem->ToolPath = FilePath::Combine(sourceDirectory, "Tools");

	contentSystem->mHistoryEnabled = contentConfig->HistoryEnabled;

	String version = Environment::GetValue<String>("versionoverride", BuildString(GetRevisionNumberString(), "-", GetChangeSetString()));

	// To avoid conflicts of assets of different versions(especially when the
	// version selector goes live) set the content folder to a unique directory
	// based upon the version number
	String revisionChangesetName = BuildString("Version-", version);

	contentSystem->ContentOutputPath =
		FilePath::Combine(contentOutputDirectory, "ContentOutput", revisionChangesetName);
	contentSystem->PrebuiltContentPath =
		FilePath::Combine(sourceDirectory, "Build", "PrebuiltContent", revisionChangesetName);
	PlasmaPrint("Content output directory '%s'\n", contentSystem->ContentOutputPath.c_str());
}

bool LoadContentLibrary(StringParam name)
{
  ZoneScoped;
  ProfileScopeFunctionArgs(name);
  ContentLibrary* library = PL::gContentSystem->Libraries.FindValue(name, nullptr);
  if (!library)
  {
    FatalEngineError("Failed to find core content library %s.\n", name.c_str());
    return false;
  }

  Status status;
  HandleOf<ResourcePackage> packageHandle = PL::gContentSystem->BuildLibrary(status, library, false);
  ResourcePackage* package = packageHandle;

  if (!status)
    return false;

  forRange (ResourceEntry& entry, package->Resources.All())
  {
    if (entry.mLibrarySource)
    {
      if (ContentEditorOptions* options = entry.mLibrarySource->has(ContentEditorOptions))
        entry.mLibrarySource->ShowInEditor = options->mShowInEditor;
      else
        entry.mLibrarySource->ShowInEditor = false;
    }
  }

  PL::gResources->LoadPackage(status, package);
  if (!status)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
  }

  return (bool)status;
}

void LoadCoreContent(Array<String>& coreLibs)
{
  ZoneScoped;
  ProfileScopeFunction();
  PL::gContentSystem->EnumerateLibraries();
  PL::gContentSystem->PlasmaCoreLibraryNames = coreLibs;

  PlasmaPrint("Loading Content...\n");

  forRange (String libraryName, coreLibs.All())
  {
    LoadContentLibrary(libraryName);
  }
}

} // namespace Plasma
