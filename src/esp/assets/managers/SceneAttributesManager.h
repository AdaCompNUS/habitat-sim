// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_ASSETS_MANAGERS_SCENEATTRIBUTEMANAGER_H_
#define ESP_ASSETS_MANAGERS_SCENEATTRIBUTEMANAGER_H_

#include "AttributesManagerBase.h"

#include "ObjectAttributesManager.h"
#include "PhysicsAttributesManager.h"

namespace esp {
namespace assets {
enum class AssetType;

namespace managers {
class SceneAttributesManager
    : public AttributesManager<PhysicsSceneAttributes::ptr> {
 public:
  SceneAttributesManager(
      assets::ResourceManager& resourceManager,
      ObjectAttributesManager::ptr objectAttributesMgr,
      PhysicsAttributesManager::ptr physicsAttributesManager);

  /**
   * @brief This will set the current physics manager attributes that is
   * governing the world that this sceneAttributesManager's scenes will be
   * created in.  This is used so that upon creation of new sceneAttributes,
   * PhysicsManagerAttributes defaults can be set in the sceneAttributes before
   * any scene-specific values are set.
   *
   * @param handle The string handle referencing the physicsManagerAttributes
   * governing the current physicsManager.
   */
  void setCurrPhysicsManagerAttributesHandle(const std::string& handle) {
    physicsManagerAttributesHandle_ = handle;
  }
  /**
   * @brief copy current @ref SimulatorConfiguration-driven values, such as file
   * paths, to make them available for scene attributes defaults.
   *
   * @param filepaths the map of file paths from the configuration object
   * @param lightSetup the config-specified light setup
   * @param frustrumCulling whether or not (semantic) scene should be
   * partitioned for culling.
   */
  void setCurrCfgVals(const std::map<std::string, std::string>& filepaths,
                      const std::string& lightSetup,
                      bool frustrumCulling) {
    cfgFilepaths_.clear();
    cfgFilepaths_.insert(filepaths.begin(), filepaths.end());
    // set lightsetup default from configuration
    cfgLightSetup_ = lightSetup;
    // set frustrum culling default from configuration
    cfgFrustrumCulling_ = frustrumCulling;
  }  // SceneAttributesManager::setCurrCfgVals

  /**
   * @brief Creates an instance of a scene template described by passed string.
   * For scene templates, this a file name.
   *
   * If a template exists with this handle, this existing template will be
   * overwritten with the newly created one if @ref registerTemplate is true.
   *
   * @param sceneAttributesHandle the origin of the desired template to be
   * created, in this case, a file name.
   * @param registerTemplate whether to add this template to the library.
   * If the user is going to edit this template, this should be false - any
   * subsequent editing will require re-registration. Defaults to true. If
   * specified as true, then this function returns a copy of the registered
   * template.
   * @return a reference to the desired template.
   */
  PhysicsSceneAttributes::ptr createAttributesTemplate(
      const std::string& sceneAttributesHandle,
      bool registerTemplate = true) override;

  /**
   * @brief Creates a an instance of scene attributes template populated with
   * default values. Assigns the @ref templateName as the template's handle,
   * and render and collision handles.
   *
   * If a template exists with this handle, the existing template will be
   * overwritten with the newly created one if @ref registerTemplate is true.
   * This method is specifically intended to directly construct an attributes
   * template for editing, and so defaults to false for @ref registerTemplate
   *
   * @param templateName Name to use for the attributes handle.
   * @param registerTemplate whether to add this template to the library.
   * If the user is going to edit this template, this should be false - any
   * subsequent editing will require re-registration. Defaults to false. If
   * specified as true, then this function returns a copy of the registered
   * template.
   * @return a reference to the desired template, or nullptr if fails.
   */
  PhysicsSceneAttributes::ptr createDefaultAttributesTemplate(
      const std::string& templateName,
      bool registerTemplate = false) override;

  /**
   * @brief Creates an instance of a scene template described by passed
   * string, which should be a reference to an existing primitive asset template
   * to be used in the construction of the scene (as render and collision
   * mesh). It returns existing instance if there is one, and nullptr if fails.
   *
   * @param primAttrTemplateHandle The handle to an existing primitive asset
   * template. Fails if does not.
   * @param registerTemplate whether to add this template to the library.
   * If the user is going to edit this template, this should be false - any
   * subsequent editing will require re-registration. Defaults to true.
   * @return a reference to the desired scene template, or nullptr if fails.
   */
  PhysicsSceneAttributes::ptr createPrimBasedAttributesTemplate(
      const std::string& primAttrTemplateHandle,
      bool registerTemplate = true);

 protected:
  /**
   * @brief Perform file-name-based attributes initialization. This is to
   * take the place of the AssetInfo::fromPath functionality, and is only
   * intended to provide default values and other help if certain mistakes
   * are made by the user, such as specifying an asset handle in json but not
   * specifying the asset type corresponding to that handle.  These settings
   * should not restrict anything, only provide defaults.
   *
   * @param attributes The AbstractPhysicsAttributes object to be configured
   * @param setFrame whether the frame should be set or not (only for render
   * assets in scenes)
   * @param fileName Mesh Handle to check.
   * @param meshTypeSetter Setter for mesh type.
   */
  void setDefaultFileNameBasedAttributes(
      PhysicsSceneAttributes::ptr attributes,
      bool setFrame,
      const std::string& meshHandle,
      std::function<void(int)> meshTypeSetter) override;
  /**
   * @brief Used Internally.  Configure newly-created attributes with any
   * default values, before any specific values are set.
   *
   * @param newAttributes Newly created attributes.
   */
  PhysicsSceneAttributes::ptr initNewAttribsInternal(
      PhysicsSceneAttributes::ptr newAttributes) override;

  /**
   * @brief This method will perform any necessary updating that is
   * attributesManager-specific upon template removal, such as removing a
   * specific template handle from the list of file-based template handles in
   * ObjectAttributesManager.  This should only be called internally.
   *
   * @param templateID the ID of the template to remove
   * @param templateHandle the string key of the attributes desired.
   */
  void updateTemplateHandleLists(
      CORRADE_UNUSED int templateID,
      CORRADE_UNUSED const std::string& templateHandle) override {}

  /**
   * @brief Scene is file-based lacking a descriptive .json, described by @ref
   * sceneFilename; populate a returned scene attributes with appropriate data.
   * This method's intended use is to support backwards compatibility for when
   * scene meshes are loaded without JSON files.
   *
   * @param sceneFilename The mesh file name
   * @param registerTemplate whether to add this template to the library or not.
   * @return a reference to the desired scene template, or nullptr if fails.
   */
  PhysicsSceneAttributes::ptr createBackCompatAttributesTemplate(
      const std::string& sceneFilename,
      bool registerTemplate = true);

  /**
   * @brief Read and parse the json file @ref sceneFilename and populate a
   * returned scene attributes with appropriate data.
   *
   * @param sceneFilename The configuration file to parse.
   * @param registerTemplate whether to add this template to the library or not.
   * @return a reference to the desired scene template, or nullptr if fails.
   */
  PhysicsSceneAttributes::ptr createFileBasedAttributesTemplate(
      const std::string& sceneFilename,
      bool registerTemplate = true);

  /**
   * @brief Add a @ref std::shared_ptr<attributesType> object to the
   * @ref templateLibrary_.  Verify that render and collision handles have been
   * set properly.  We are doing this since these values can be modified by the
   * user.
   *
   * @param sceneAttributesTemplate The attributes template.
   * @param sceneAttributesHandle The key for referencing the template in the
   * @ref templateLibrary_.
   * @return The index in the @ref templateLibrary_ of object
   * template.
   */

  int registerAttributesTemplateFinalize(
      PhysicsSceneAttributes::ptr sceneAttributesTemplate,
      const std::string& sceneAttributesHandle) override;

  /**
   * @brief Any scene-attributes-specific resetting that needs to happen on
   * reset.
   */
  void resetFinalize() override {}

  /**
   * @brief This function will assign the appropriately configured function
   * pointer for the copy constructor as required by
   * AttributesManager<PhysicsSceneAttributes::ptr>
   */
  void buildCtorFuncPtrMaps() override {
    this->copyConstructorMap_["PhysicsSceneAttributes"] =
        &SceneAttributesManager::createAttributesCopy<
            assets::PhysicsSceneAttributes>;
  }  // SceneAttributesManager::buildCtorFuncPtrMaps

  // instance vars

  /**
   * @brief Reference to ObjectAttributesManager to give access to setting
   * object template library using paths specified in SceneAttributes json
   */
  ObjectAttributesManager::ptr objectAttributesMgr_ = nullptr;
  /**
   * @brief Reference to PhysicsAttributesManager to give access to default
   * physics manager attributes settings when sceneAttributes are created.
   */
  PhysicsAttributesManager::ptr physicsAttributesManager_ = nullptr;

  /**
   * @brief Current file paths based on @ref SimulatorConfiguration settings.
   * Paths can be overridden by json-specified values.
   */
  std::map<std::string, std::string> cfgFilepaths_;

  /**
   * @brief Current lighting default value based on current @ref
   * SimulatorConfiguration settings. Potentially overridden by scene-specific
   * json.
   */
  std::string cfgLightSetup_;

  /**
   * @brief Current frustrum culling setting based on current @ref
   * SimulatorConfiguration settings. Potentially overridden by scene-specific
   * json.
   */
  bool cfgFrustrumCulling_ = false;

  /**
   * @brief Name of currently used physicsManagerAttributes
   */
  std::string physicsManagerAttributesHandle_ = "";

 public:
  ESP_SMART_POINTERS(SceneAttributesManager)

};  // SceneAttributesManager

}  // namespace managers
}  // namespace assets
}  // namespace esp

#endif  // ESP_ASSETS_MANAGERS_SCENEATTRIBUTEMANAGER_H_