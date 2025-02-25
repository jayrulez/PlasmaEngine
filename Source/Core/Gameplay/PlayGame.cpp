// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void LoadResourcePackageRelative(StringParam baseDirectory, StringParam libraryName)
{
  String path = FilePath::Combine(baseDirectory, libraryName);
  String fileName = BuildString(libraryName, ".pack");
  String packageFile = FilePath::Combine(path, fileName);

  if (!FileExists(packageFile))
  {
    FatalEngineError("Failed to find needed content package. %s", libraryName.c_str());
  }

  PL::gResources->LoadPackageFile(packageFile);
}

void LoadGamePackages(StringParam projectFile, Cog* projectCog)
{
  String projectDirectory = FilePath::GetDirectoryPath(projectFile);
  LoadResourcePackageRelative(projectDirectory, "FragmentCore");
  LoadResourcePackageRelative(projectDirectory, "Loading");
  LoadResourcePackageRelative(projectDirectory, "PlasmaCore");
  LoadResourcePackageRelative(projectDirectory, "UiWidget");
  LoadResourcePackageRelative(projectDirectory, "EditorUi");
  LoadResourcePackageRelative(projectDirectory, "Editor");


  ProjectSettings* project = projectCog->has(ProjectSettings);

  forRange (String library, project->ExtraLibraries.All())
  {
    LoadResourcePackageRelative(projectDirectory, library);
  }

  LoadResourcePackageRelative(projectDirectory, project->ProjectName);
}

void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog)
{
  PlasmaPrint("Creating Game\n");

  // Set the store name based on the project name.
  ProjectSettings* project = projectCog->has(ProjectSettings);
  ObjectStore::GetInstance()->SetStoreName(project->ProjectName);

  // Make sure scripts in the project are compiled
  LightningManager::GetInstance()->TriggerCompileExternally();

  // Send after compiling since graphics uses this event to know to stop
  // displaying the splash/loading screen
  ObjectEvent event(projectCog);
  PL::gEngine->DispatchEvent(Events::ProjectLoaded, &event);

  PlasmaPrint("Creating game...\n");

  IntVec2 intViewportSize = mainWindow->GetClientSize();
  Vec2 mainViewportSize = Pixels(float(intViewportSize.x), float(intViewportSize.y));

  RootWidget* rootWidget = new RootWidget(mainWindow);
  rootWidget->SetSize(mainViewportSize);

  // In game mode only one widget that has the viewport
  GameWidget* gameWidget = new GameWidget(rootWidget);
  gameWidget->TakeFocus();
  gameWidget->SetSize(mainViewportSize);

  // Create the GameSession
  GameSession* game = PL::gEngine->CreateGameSession();
  game->mMainWindow = mainWindow;
  game->SetInEditor(false);

  game->mGameWidget = gameWidget;
  gameWidget->SetGameSession(game);

  game->Start();

  // Run all parsed command-line arguments once after the game is fully loaded.
  CommandManager* commandManager = CommandManager::GetInstance();
  commandManager->RunParsedCommandsDelayed();
}

} // namespace Plasma
