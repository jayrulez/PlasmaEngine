// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Plasma
{

void DispatchCodeTranslatorScriptError(StringParam eventId,
                                       StringParam shortMessage,
                                       StringParam fullMessage,
                                       const Lightning::CodeLocation& location)
{
  // This should only happen when a composite has a lightning error. Figure out how
  // to report later?
  if (location.CodeUserData == nullptr)
    return;

  Resource* resource = (Resource*)location.CodeUserData;
  DocumentResource* documentResource = Type::DynamicCast<DocumentResource*>(resource);
  if (resource == nullptr)
    return;

  ScriptEvent e;
  e.Script = documentResource;
  e.Message = shortMessage;
  e.Location = location;
  PL::gResources->DispatchEvent(eventId, &e);

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

// Simple error callback function for the compositor.
// Sends out the event to display an inline error in the script file.
void OnLightningFragmentCompilationError(Lightning::ErrorEvent* e)
{
  String shortMessage = e->ExactError;
  String fullMessage = e->GetFormattedMessage(Lightning::MessageFormat::Python);
  DispatchCodeTranslatorScriptError(Events::SyntaxError, shortMessage, fullMessage, e->Location);
}

void OnLightningFragmentTranslationError(TranslationErrorEvent* e)
{
  String fullMessage = e->GetFormattedMessage(Lightning::MessageFormat::Python);
  DispatchCodeTranslatorScriptError(Events::SyntaxError, e->mShortMessage, fullMessage, e->mLocation);
}

TranslatedShaderScriptEditor::TranslatedShaderScriptEditor(Composite* parent) : ScriptEditor(parent)
{
  // mShaderGenerator = nullptr;
  ConnectThisTo(LightningManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnBuild);
}

TranslatedShaderScriptEditor::~TranslatedShaderScriptEditor()
{
  // delete mShaderGenerator;
}

void TranslatedShaderScriptEditor::Build()
{
  // if(mShaderGenerator != nullptr)
  //{
  //  delete mShaderGenerator;
  //  mShaderGenerator = nullptr;
  //}
  // mShaderGenerator = new SimpleLightningShaderGenerator(nullptr);

  Vec2 scrollPercent = GetScrolledPercentage();
  SetAllText(OnTranslate());
  SetScrolledPercentage(scrollPercent);
}
void TranslatedShaderScriptEditor::OnBuild(Event* e)
{
  Build();
}

String TranslatedShaderScriptEditor::OnTranslate()
{
  return "";
}

// void
// TranslatedShaderScriptEditor::CompileAndTranslateFragments(SimpleLightningShaderGenerator&
// shaderGenerator)
//{
//  // This is all kinda legacy now and should be cleaned up at some
//  point.
//  // This is mainly for testing my simpler stuff instead of graphic's more
//  complicated pipeline. EventConnect(&shaderGenerator,
//  Events::TranslationError, &OnLightningFragmentTranslationError);
//  EventConnect(&shaderGenerator, Lightning::Events::CompilationError,
//  &OnLightningFragmentCompilationError);
//
//  // Remove all old code
//  shaderGenerator.ClearFragmentsProjectAndLibrary();
//  // For now load the extensions from the data directory (otherwise
//  there's an issue with putting
//  // the resources in the core library as resources don't really do anything
//  special with resource libraries yet) MainConfig* mainConfig =
//  PL::gEngine->GetConfigCog()->has(MainConfig); String settingsDir =
//  FilePath::Combine(mainConfig->DataDirectory, "LightningFragmentSettings");
//  String extensionsDir = FilePath::Combine(mainConfig->DataDirectory,
//  "LightningFragmentExtensions"); shaderGenerator.LoadSettings(settingsDir);
//  shaderGenerator.SetupDependencies(extensionsDir);
//
//  // Iterate over all fragments and add them to the compositor
//  LightningFragmentManager* manager = LightningFragmentManager::GetInstance();
//  LightningFragmentManager::ResourceIdMapType::valuerange range =
//  manager->ResourceIdMap.Values(); for(; !range.Empty(); range.PopFront())
//  {
//    LightningFragment* fragment = (LightningFragment*)range.Front();
//
//    String fileName = fragment->Name;
//    if(fragment->mContentItem)
//    {
//      if(fragment->mContentItem->has(ResourceTemplate))
//        continue;
//      fileName = fragment->mContentItem->GetFullPath();
//    }
//
//    // Add the fragment* itself as the user data
//    // (the error callback function uses this to display errors on the script)
//    shaderGenerator.AddFragmentCode(fragment->mText, fileName, fragment);
//  }
//
//  shaderGenerator.CompileAndTranslateFragments();
//}
//
// void
// TranslatedShaderScriptEditor::TranslateMaterial(SimpleLightningShaderGenerator&
// shaderGenerator, Material* material)
//{
//  CompileAndTranslateFragments(shaderGenerator);
//
//  LightningShaderLibrary* fragmentLibrary = shaderGenerator.mFragmentLibraryRef;
//  if(fragmentLibrary == nullptr)
//    return;
//
//  AutoDeclare(fragmentTypeRange, fragmentLibrary->mTypes.Values());
//
//  // For now this assumes that all materials blocks on the material are lightning
//  fragments LightningShaderDefinition shaderDef; shaderDef.mShaderName =
//  material->Name; for(size_t i = 0; i < material->mMaterialBlocks.Size(); ++i)
//  {
//    MaterialBlock* block = material->mMaterialBlocks[i];
//    String blockName = LightningVirtualTypeId(block)->Name;
//    shaderDef.mFragmentTypes.PushBack(fragmentLibrary->mTypes[blockName]);
//  }
//
//  ShaderDefinitionSettings& shaderDefSettings =
//  shaderGenerator.mSettings->mShaderDefinitionSettings;
//
//  // Composite the lightning shader for this material
//  shaderGenerator.ComposeShader(shaderDef);
//  shaderGenerator.CompileAndTranslateShaders();
//}

FragmentFileTranslatorScriptEditor::FragmentFileTranslatorScriptEditor(Composite* parent) :
    TranslatedShaderScriptEditor(parent)
{
  mFragment = nullptr;
}

void FragmentFileTranslatorScriptEditor::SetResource(LightningFragment* fragment)
{
  mFragment = fragment;
  ListenForModified(mFragment);
}

String FragmentFileTranslatorScriptEditor::OnTranslate()
{
  return String();
  // SimpleLightningShaderGenerator& shaderGenerator = *mShaderGenerator;
  // CompileAndTranslateFragments(shaderGenerator);
  //
  // LightningShaderLibrary* library = shaderGenerator.mFragmentLibraryRef;
  //// Make sure the library successfully compiled
  // if(library == nullptr)
  //  return "Failed to compile";
  //
  // StringBuilder builder;
  //// Append a string that is all of the types who originated from this
  /// fragment file
  // AutoDeclare(typeRange, library->mTypes.Values());
  // for(; !typeRange.Empty(); typeRange.PopFront())
  //{
  //  ShaderType* type = typeRange.Front();
  //  if(type->mFileName == mFragment->LoadPath)
  //  {
  //    ShaderTypeTranslation result;
  //    mShaderGenerator->mTranslator->BuildFinalShader(type, result);
  //    builder.Append(result.mTranslation);
  //  }
  //}
  //
  // String result = builder.ToString();
  //// If we didn't generate any strings then return that we couldn't find the
  /// translation
  // if(result.Empty())
  //  result = "Not found";
  //
  // return result;
}

LightningCompositorScriptEditor::LightningCompositorScriptEditor(Composite* parent) : TranslatedShaderScriptEditor(parent)
{
  mMaterial = nullptr;
}

void LightningCompositorScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  ListenForModified(mMaterial);
}

String LightningCompositorScriptEditor::OnTranslate()
{
  return String();
  // SimpleLightningShaderGenerator shaderGenerator;
  // TranslateMaterial(*mShaderGenerator, mMaterial);
  //
  //// Find the shader that resulted from the given material. This has to do a
  /// search of the project to / find which lightning file was created for the
  /// material. As this is not a normal operation in / translation/compositing
  /// and is only used for debugging I decided a search was acceptable.
  // String result = mShaderGenerator->FindLightningShaderString(mMaterial->Name);
  // if(result.Empty())
  //  result = "Failed to compose shader";
  // return result;
}

TranslatedLightningCompositorScriptEditor::TranslatedLightningCompositorScriptEditor(Composite* parent) :
    TranslatedShaderScriptEditor(parent)
{
  mMaterial = nullptr;
  mDisplayMode = TranslationDisplayMode::Full;
}

void TranslatedLightningCompositorScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  ListenForModified(mMaterial);
}

String TranslatedLightningCompositorScriptEditor::OnTranslate()
{
  return String();
  // SimpleLightningShaderGenerator& shaderGenerator = *mShaderGenerator;
  // TranslateMaterial(shaderGenerator, mMaterial);
  // if(shaderGenerator.mShaderLibraryRef == nullptr)
  //  return "Failed to compile";
  //
  // LightningShaderDefinition* result =
  // shaderGenerator.FindShaderResults(mMaterial->Name); if(result == nullptr)
  //  return "Failed to find";
  //
  // String vertexShaderName =
  // result->mShaderData[FragmentType::Vertex].mLightningClassName; String
  // geometryShaderName =
  // result->mShaderData[FragmentType::Geometry].mLightningClassName; String
  // pixelShaderName = result->mShaderData[FragmentType::Pixel].mLightningClassName;
  //
  // ShaderType* vertexShaderType =
  // shaderGenerator.mShaderLibraryRef->FindType(vertexShaderName); ShaderType*
  // geometryShaderType =
  // shaderGenerator.mShaderLibraryRef->FindType(geometryShaderName);
  // ShaderType* pixelShaderType =
  // shaderGenerator.mShaderLibraryRef->FindType(pixelShaderName);
  //
  // ShaderTypeTranslation vertexTranslation;
  // ShaderTypeTranslation geometryTranslation;
  // ShaderTypeTranslation pixelTranslation;
  //
  // BaseShaderTranslator* translator = shaderGenerator.mTranslator;
  // translator->BuildFinalShader(vertexShaderType, vertexTranslation);
  // translator->BuildFinalShader(geometryShaderType, geometryTranslation);
  // translator->BuildFinalShader(pixelShaderType, pixelTranslation);
  //
  // if(mDisplayMode == TranslationDisplayMode::Vertex)
  //  return vertexTranslation.mTranslation;
  // if(mDisplayMode == TranslationDisplayMode::Geometry)
  //  return geometryTranslation.mTranslation;
  // if(mDisplayMode == TranslationDisplayMode::Pixel)
  //  return pixelTranslation.mTranslation;
  //
  // String combinedTranslation = BuildString(vertexTranslation.mTranslation,
  // pixelTranslation.mTranslation); return combinedTranslation;
}

void TranslatedLightningCompositorScriptEditor::SetDisplayMode(TranslationDisplayMode::Enum displayMode)
{
  mDisplayMode = displayMode;
}

BaseSplitScriptEditor::BaseSplitScriptEditor(Composite* parent) : Composite(parent)
{
  // Can't called setup now as virtual functions won't properly be setup yet
}

void BaseSplitScriptEditor::Setup()
{
  SetLayout(CreateRowLayout());

  mSourceEditor = new ScriptEditor(this);
  mSplitter = new Splitter(this);
  SetTranslatedEditor();

  mSourceEditor->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);
  mSplitter->SetSizing(SizeAxis::X, SizePolicy::Fixed, 4);
  mTranslatedEditor->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);

  ConnectThisTo(PL::gEditor, Events::Save, OnSaveCheck);
  ConnectThisTo(LightningManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnBuild);
  ConnectThisTo(mTranslatedEditor, Events::LeftMouseDown, OnLeftMouseDown);
}

void BaseSplitScriptEditor::SetTranslatedEditor()
{
}

void BaseSplitScriptEditor::SaveCheck()
{
}
//
// void BaseSplitScriptEditor::BuildFinalShader(ShaderTypeTranslation&
// shaderResult)
//{
//
//}

void BaseSplitScriptEditor::Build()
{
}

void BaseSplitScriptEditor::SetLexer(uint lexer)
{
  mSourceEditor->SetLexer(Lexer::Lightning);
  mTranslatedEditor->SetLexer(lexer);
}

void BaseSplitScriptEditor::OnSaveCheck(SavingEvent* e)
{
  SaveCheck();
}

void BaseSplitScriptEditor::OnBuild(Event* e)
{
  Build();
}

void BaseSplitScriptEditor::OnLeftMouseDown(MouseEvent* e)
{
  //// Build the final shader string
  // ShaderTypeTranslation result;
  // BuildFinalShader(result);
  //
  //// Find where the destination position mapped to
  // int destLinePos = mTranslatedEditor->GetCurrentPosition();
  // CodeRangeMapping* subRange = FindRange(destLinePos, &result.mRoot);
  // if(subRange == nullptr)
  //  return;
  //
  //// Find the source file for the current range (it could be in a fragment or
  /// shader so search both)
  // SimpleLightningShaderGenerator* shaderGenerator =
  // mTranslatedEditor->mShaderGenerator; LightningShaderProject& fragmentProject =
  // shaderGenerator->mFragmentProject; LightningShaderProject& shaderProject =
  // shaderGenerator->mShaderProject; for(size_t i = 0; i <
  // mTranslatedEditor->mShaderGenerator->mFragmentProject.mCodeEntries.Size();
  // ++i)
  //{
  //  LightningShaderProject::CodeEntry& entry =
  //  mTranslatedEditor->mShaderGenerator->mFragmentProject.mCodeEntries[i];
  //  if(entry.mCodeLocation == subRange->mSourceFile)
  //    mSourceEditor->SetAllText(entry.mCode);
  //}
  // for(size_t i = 0; i < shaderProject.mCodeEntries.Size(); ++i)
  //{
  //  LightningShaderProject::CodeEntry& entry = shaderProject.mCodeEntries[i];
  //  if(entry.mCodeLocation == subRange->mSourceFile)
  //    mSourceEditor->SetAllText(entry.mCode);
  //}
  //
  //// Select the location in the text editor of the lightning script
  // int sourceFirstLineStart = subRange->mSourcePositionStart;
  // int sourceLastLineEnd = subRange->mSourcePositionEnd;
  // mSourceEditor->GotoAndSelect(sourceFirstLineStart, sourceLastLineEnd);
}
//
// CodeRangeMapping* BaseSplitScriptEditor::FindRange(int positionWithinParent,
// CodeRangeMapping* current)
//{
//  for(size_t i = 0; i < current->mChildren.Size(); ++i)
//  {
//    CodeRangeMapping* child = &current->mChildren[i];
//    // If this position is not within the child's position range then skip it
//    if(positionWithinParent < child->mDestPositionStart ||
//    child->mDestPositionEnd < positionWithinParent)
//      continue;
//
//    // Otherwise we're within this child so recurse into it, but first
//    // compute the relative offset for the child from our current position.
//    return FindRange(positionWithinParent - child->mDestPositionStart, child);
//  }
//  return current;
//}

FragmentSplitScriptEditor::FragmentSplitScriptEditor(Composite* parent) : BaseSplitScriptEditor(parent), mFragment()
{
  // This has to be called here so that the virtual functions will be properly
  // set
  Setup();
}

void FragmentSplitScriptEditor::SetTranslatedEditor()
{
  mTranslatedEditor = new FragmentFileTranslatorScriptEditor(this);
}

void FragmentSplitScriptEditor::SaveCheck()
{
  if (mSourceEditor->IsModified())
    mFragment->mText = mSourceEditor->GetAllText();
}

void FragmentSplitScriptEditor::Build()
{
  mSourceEditor->SetAllText(mFragment->mText);
  mTranslatedEditor->Build();
}
//
// void FragmentSplitScriptEditor::BuildFinalShader(ShaderTypeTranslation&
// shaderResult)
//{
//  LightningShaderLibrary* library =
//  mTranslatedEditor->mShaderGenerator->mFragmentLibraryRef; ShaderType* type =
//  library->FindType(mFragment->Name);
//
//  mTranslatedEditor->mShaderGenerator->mTranslator->BuildFinalShader(type,
//  shaderResult, true, false);
//}

void FragmentSplitScriptEditor::SetResource(LightningFragment* fragment)
{
  mFragment = fragment;
  FragmentFileTranslatorScriptEditor* editor = (FragmentFileTranslatorScriptEditor*)mTranslatedEditor;
  editor->SetResource(fragment);
}

MaterialSplitScriptEditor::MaterialSplitScriptEditor(Composite* parent) : BaseSplitScriptEditor(parent), mMaterial()
{
  mDisplayMode = TranslationDisplayMode::Pixel;
  // This has to be called here so that the virtual functions will be properly
  // set
  Setup();
}

void MaterialSplitScriptEditor::SetTranslatedEditor()
{
  TranslatedLightningCompositorScriptEditor* editor = new TranslatedLightningCompositorScriptEditor(this);
  editor->mDisplayMode = mDisplayMode;
  mTranslatedEditor = editor;
}

void MaterialSplitScriptEditor::Build()
{
  mTranslatedEditor->Build();
}

// void MaterialSplitScriptEditor::BuildFinalShader(ShaderTypeTranslation&
// shaderResult)
//{
//  LightningShaderDefinition* def =
//  mTranslatedEditor->mShaderGenerator->FindShaderResults(mMaterial->Name);
//
//  // Get the shader name (currently the translation display mode doesn't map
//  to the fragment type enum) String shaderName; if(mDisplayMode ==
//  TranslationDisplayMode::Pixel)
//    shaderName = def->mShaderData[FragmentType::Pixel].mLightningClassName;
//  if(mDisplayMode == TranslationDisplayMode::Vertex)
//    shaderName = def->mShaderData[FragmentType::Vertex].mLightningClassName;
//
//  ShaderType* type =
//  mTranslatedEditor->mShaderGenerator->mShaderLibraryRef->FindType(shaderName);
//  mTranslatedEditor->mShaderGenerator->mTranslator->BuildFinalShader(type,
//  shaderResult, true, true);
//}

void MaterialSplitScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  TranslatedLightningCompositorScriptEditor* editor = (TranslatedLightningCompositorScriptEditor*)mTranslatedEditor;
  editor->SetResource(material);
}

void MaterialSplitScriptEditor::SetDisplayMode(TranslationDisplayMode::Enum displayMode)
{
  mDisplayMode = displayMode;
  TranslatedLightningCompositorScriptEditor* editor = (TranslatedLightningCompositorScriptEditor*)mTranslatedEditor;
  editor->SetDisplayMode(mDisplayMode);
}

// Tries to find a script window of a given name, if it cannot be found then it
// is created.
template <typename ScriptEditorType>
ScriptEditorType* CreateScriptWindow(Editor* editor, StringParam name, uint lexerType)
{
  // get the center window (where we add all tabs to)
  Window* centerWindow = editor->GetCenterWindow();

  // find the tab for this translation if we already had it open
  WindowTabEvent event;
  event.Name = name;
  centerWindow->DispatchBubble(Events::TabFind, &event);

  ScriptEditorType* scriptEditor = nullptr;
  if (event.TabWidgetFound)
  {
    scriptEditor = (ScriptEditorType*)event.TabWidgetFound;
  }
  else
  {
    // if it doesn't exist then create the window, set the lexers,
    // give the window reference to the source script (so it can save it)
    scriptEditor = new ScriptEditorType(centerWindow);
    scriptEditor->SetName(name);
    PL::gEditor->AddManagedWidget(scriptEditor, DockArea::Center, true);
  }

  scriptEditor->SetLexer(lexerType);
  return scriptEditor;
}

struct ShaderFileBuilder
{
  String CreateFileWithContents(StringParam fileContents, StringParam className)
  {
    return fileContents;
  }

  Resource* GetResourceByName(StringParam name)
  {
    return LightningFragmentManager::GetInstance()->GetResource(name, ResourceNotFound::ReturnNull);
  }
};

template <typename Functor>
void RunCodeTranslator(
    Editor* editor, Space* space, CodeTranslator* translator, Functor customFileBuilder, StringParam rerunCommand)
{
  // get the center window (where we add all tabs to)
  Window* centerWindow = editor->GetCenterWindow();

  // get all of the translated files
  HashMap<String, String> files;
  translator->Translate(files);

  auto fileRange = files.All();
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    // get the item name
    auto item = fileRange.Front();
    // get the translated code and file name
    String translatedCode = item.second;
    String fileName = item.first;
    // the file name is the full file path, so strip of all of the path
    StringRange found = fileName.FindLastOf('\\');
    String name = fileName.SubString(found.End(), fileName.End());
    // and then strip of the .z
    found = name.FindLastOf('.');
    String className = name.SubString(name.Begin(), found.Begin());

    // Try to find the original file's tab. We don't want all script files
    // to open, only open the ones that we were editing
    //(aka if we find the tab for the file then we open)
    WindowTabEvent* origFileEvent = new WindowTabEvent();
    Resource* resource = customFileBuilder.GetResourceByName(className);
    origFileEvent->SearchObject = resource;
    origFileEvent->Name = className;
    centerWindow->DispatchBubble(Events::TabFind, origFileEvent);
    if (origFileEvent->TabWidgetFound == nullptr)
      continue;

    // make a name for the tab with fileName : Translated
    name = String::Format("%s : Translated", name.c_str());

    // get the script that this represented
    DocumentResource* script = (DocumentResource*)resource;

    // find the tab for this translation if we already had it open
    WindowTabEvent event;
    event.Name = name;
    centerWindow->DispatchBubble(Events::TabFind, &event);

    CodeSplitWindow* splitWindow;
    if (event.TabWidgetFound)
    {
      splitWindow = (CodeSplitWindow*)event.TabWidgetFound;
    }
    else
    {
      // if it doesn't exist then create the window, set the lexers,
      // give the window reference to the source script (so it can save it)
      splitWindow = new CodeSplitWindow(centerWindow);
      splitWindow->mCommandToRunOnSave = rerunCommand;
      splitWindow->SetName(name);
      splitWindow->mSourceResource = script;
      splitWindow->SetLexers(translator);
      PL::gEditor->AddManagedWidget(splitWindow, DockArea::Center, true);
    }
    // make sure the scroll bar stays approximately where it is
    Vec2 scrollPercent = splitWindow->mSourceText->GetScrolledPercentage();
    splitWindow->mSourceText->SetAllText(script->LoadTextData());
    splitWindow->mSourceText->SetScrolledPercentage(scrollPercent);

    String fileText = customFileBuilder.CreateFileWithContents(translatedCode, className);

    scrollPercent = splitWindow->mTranslatedText->GetScrolledPercentage();
    splitWindow->mTranslatedText->SetAllText(fileText);
    splitWindow->mTranslatedText->SetScrolledPercentage(scrollPercent);
  }
}

CodeTranslatorListener::CodeTranslatorListener()
{
  ConnectThisTo(PL::gEditor, "ComposeLightningMaterial", OnComposeLightningMaterial);
  ConnectThisTo(PL::gEditor, "TranslateLightningFragment", OnTranslateLightningFragment);
  ConnectThisTo(PL::gEditor, "TranslateLightningPixelMaterial", OnTranslateLightningPixelFragment);
  ConnectThisTo(PL::gEditor, "TranslateLightningGeometryMaterial", OnTranslateLightningGeometryFragment);
  ConnectThisTo(PL::gEditor, "TranslateLightningVertexMaterial", OnTranslateLightningVertexFragment);

  // ConnectThisTo(PL::gEditor, "TranslateLightningFragment",
  // OnTranslateLightningFragmentWithLineNumbers); ConnectThisTo(PL::gEditor,
  // "TranslateLightningPixelMaterial",
  // OnTranslateLightningPixelFragmentWithLineNumbers); ConnectThisTo(PL::gEditor,
  // "TranslateLightningVertexMaterial",
  // OnTranslateLightningVertexFragmentWithLineNumbers);
}

template <typename EditorType>
void CreateFragmentTranslationWindow(ObjectEvent* e, StringParam baseWindowName)
{
  Editor* editor = PL::gEditor;
  LightningFragment* fragment = (LightningFragment*)e->Source;
  EditorType* scriptEditor =
      CreateScriptWindow<EditorType>(editor, BuildString(baseWindowName, fragment->Name), Lexer::Shader);
  scriptEditor->SetResource(fragment);
  scriptEditor->Build();
}

template <typename EditorType>
void CreateMaterialTranslationWindow(ObjectEvent* e,
                                     TranslationDisplayMode::Enum displayMode,
                                     StringParam baseWindowName)
{
  Editor* editor = PL::gEditor;
  Material* material = (Material*)e->Source;
  EditorType* scriptEditor =
      CreateScriptWindow<EditorType>(editor, BuildString(baseWindowName, material->Name), Lexer::Shader);
  scriptEditor->SetDisplayMode(displayMode);
  scriptEditor->SetResource(material);
  scriptEditor->Build();
}

void CodeTranslatorListener::OnComposeLightningMaterial(ObjectEvent* e)
{
  Editor* editor = PL::gEditor;
  Material* material = (Material*)e->Source;
  LightningCompositorScriptEditor* scriptEditor = CreateScriptWindow<LightningCompositorScriptEditor>(
      editor, BuildString("CompositedMat", material->Name), Lexer::Lightning);
  scriptEditor->SetResource(material);
  scriptEditor->Build();
}

void CodeTranslatorListener::OnTranslateLightningFragment(ObjectEvent* e)
{
  CreateFragmentTranslationWindow<FragmentFileTranslatorScriptEditor>(e, "Translated");
}

void CodeTranslatorListener::OnTranslateLightningPixelFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedLightningCompositorScriptEditor>(
      e, TranslationDisplayMode::Pixel, "TranslatedPixelMat");
}

void CodeTranslatorListener::OnTranslateLightningGeometryFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedLightningCompositorScriptEditor>(
      e, TranslationDisplayMode::Geometry, "TranslatedGeometryMat");
}

void CodeTranslatorListener::OnTranslateLightningVertexFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedLightningCompositorScriptEditor>(
      e, TranslationDisplayMode::Vertex, "TranslatedVertexMat");
}

void CodeTranslatorListener::OnTranslateLightningFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateFragmentTranslationWindow<FragmentSplitScriptEditor>(e, "Translated");
}

void CodeTranslatorListener::OnTranslateLightningPixelFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<MaterialSplitScriptEditor>(e, TranslationDisplayMode::Pixel, "TranslatedPixelMat");
}

void CodeTranslatorListener::OnTranslateLightningVertexFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<MaterialSplitScriptEditor>(e, TranslationDisplayMode::Vertex, "TranslatedVertexMat");
}

void CreateShaderTranslationDebugHelper(Editor* editor)
{
  ShaderTranslationDebugHelper* translatorWindow = new ShaderTranslationDebugHelper(editor);
  editor->AddManagedWidget(translatorWindow, DockArea::Center, true);
  translatorWindow->SetSize(Vec2(500, 500));
}

void BindCodeTranslatorCommands(Cog* configCog, CommandManager* commands)
{
  // Ideally change this to add as a component later
  PL::gEditor->mCodeTranslatorListener = new CodeTranslatorListener();
  commands->AddCommand("DebugShaderTranslation", BindCommandFunction(CreateShaderTranslationDebugHelper));
}

}
