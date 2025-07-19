//
// Created by PinkySmile on 11/02/23.
//

#include "Game.hpp"
#include "SceneManager.hpp"

namespace SpiralOfFate
{
	void SceneManager::update()
	{
		if (this->_loading) {
			if (this->_scene)
				this->_scene->update();
			return;
		}
		this->_oldScene.reset();
		if (!this->_nextScene.name.empty() && this->_currentScene != this->_nextScene.name)
			this->_applySwitchScene();
		else if (this->_scene)
			this->_scene->update();
	}

	void SceneManager::render() const
	{
		assert_exp(this->_scene || this->_oldScene);
		(this->_scene ? this->_scene : this->_oldScene)->render();
	}

	void SceneManager::consumeEvent(const sf::Event &event)
	{
		assert_exp(this->_scene || this->_oldScene);
		(this->_scene ? this->_scene : this->_oldScene)->consumeEvent(event);
	}

	void SceneManager::switchScene(const std::string &name, SceneArguments *args, bool force)
	{
		if (!force)
			assert_exp(name != this->_currentScene);
		assert_exp(!this->_loading);
		assert_msg(this->_factory.contains(name), "Scene '" + name + "' doesn't exist");
		assert_exp(!this->_factory[name].needLoading || this->_factory.contains("loading"));
		delete this->_nextScene.args;
		this->_nextScene.args = args;
		this->_nextScene.name = name;
		if (!this->_scene)
			this->_applySwitchScene();
	}

	SceneManager::SceneConstructor SceneManager::registerScene(
		const std::string &name,
		const std::function<IScene *(SceneArguments *)> &constructor,
		bool requireLoadingScene
	)
	{
		auto old = this->_factory[name];

		assert_exp(name != "loading" || !requireLoadingScene);
		this->_factory[name].callback = constructor;
		this->_factory[name].needLoading = requireLoadingScene;
		return old;
	}

	bool SceneManager::isLoading()
	{
		return this->_loading;
	}

	std::pair<std::string, IScene *> SceneManager::getCurrentScene()
	{
		return {
			this->_currentScene,
			&*this->_scene
		};
	}

	void SceneManager::_applySwitchScene()
	{
		auto &factory = this->_factory[this->_nextScene.name];
		auto cb = [this](IScene *result){
			this->_oldScene = this->_scene;
			assert_exp(result);
			this->_scene.reset(result);
			this->_loading = false;
		};

		if (factory.needLoading) {
			LoadingArguments arg{factory.callback};

			this->_loading = true;
			arg.onLoadingFinished = cb;
			arg.args = this->_nextScene.args;
			this->_scene.reset(this->_factory["loading"].callback(&arg));
		} else
			cb(factory.callback(this->_nextScene.args));
		this->_currentScene = this->_nextScene.name;
		this->_nextScene.name.clear();
	}
}