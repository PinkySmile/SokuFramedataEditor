#include <iostream>
#include <memory>
#include <thread>
#include <TGUI/TGUI.hpp>
#include <SoFGV.hpp>
#include "EditableObject.hpp"
#include <package.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ActionName.hpp"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#define mkdir(path, mode) mkdir(path)
#endif

using namespace SpiralOfFate;

void	displayPalEditor(std::unique_ptr<EditableObject> &object, tgui::Gui &gui, bool force = false);

auto c = std::make_shared<bool>(false);
float updateTimer = 0;
float timer = 0;
std::string loadedPath;
bool colorPickDrag = false;
bool hovering = false;
bool normalDrag = false;
bool canDrag = false;
bool dragStart = false;
bool dragUp = false;
bool dragDown = false;
bool dragLeft = false;
bool dragRight = false;
bool updateAnyway = false;
bool spriteSelected = false;
bool quitRequest = false;
tgui::Color normalColor;
tgui::Button::Ptr boxButton;
Vector2i mouseStart;
Vector2i lastMouse;
static bool displayHitboxes = true;
static int selectedColor = 0;
ShadyCore::Schema::Sequence::BBox *selectedBox;
std::array<tgui::Button::Ptr, 8> resizeButtons;
std::string swr;
std::string soku;
std::string soku2;
std::vector<std::string> extra;
std::vector<ShadyCore::Package *> extraPackages;
sf::Color bgColor = sf::Color::Black;
sf::Image selectedColorMaskImage;
sf::Sprite selectedColorMaskSprite;
sf::Texture selectedColorMaskTexture;
sf::Texture tempTexture;
ShadyCore::Schema tempSchema;
std::list<std::function<void()>> uiTasks;
std::map<unsigned, std::string> actionNames;

struct {
	bool display = false;
	bool front = true;
	bool play = true;
	SpiralOfFate::Vector2i offset = {0, 0};
	std::unique_ptr<OverlayObject> object;
} overlayData;

struct {
	ShadyCore::Schema *schema = nullptr;
	ShadyCore::Palette *palette = nullptr;
	std::string palName;
	std::string chr;
} editSession;

extern uint32_t getOrCreateImage(ShadyCore::Schema &schema, const std::string_view& name);

void	arrangeButtons(EditableObject *object)
{
	auto *data = object ? &object->_moves.at(object->_action)[object->_actionBlock][object->_animation] : nullptr;
	Box box = spriteSelected ? Box{
		{
			static_cast<int>(-data->frame->offsetX),
			static_cast<int>(-data->frame->offsetY)
		}, {
			static_cast<unsigned int>(data->frame->texWidth * (data->frame->blendOptions.scaleX ? data->frame->blendOptions.scaleX : 200) / 100.f),
			static_cast<unsigned int>(data->frame->texHeight * (data->frame->blendOptions.scaleY ? data->frame->blendOptions.scaleY : 200) / 100.f)
		}
	} : Box{
		{selectedBox->left, selectedBox->up},
		{
			static_cast<unsigned int>(selectedBox->right - selectedBox->left),
			static_cast<unsigned int>(selectedBox->down - selectedBox->up)
		}
	};

	for (int i = 0; i < 8; i++) {
		Vector2i pos;
		auto resizeButton = resizeButtons[i];

		if (i == 0 || i == 3 || i == 5)
			pos.x = box.pos.x - resizeButton->getSize().x;
		else if (i == 2 || i == 4 || i == 7)
			pos.x = box.pos.x + box.size.x;
		else
			pos.x = box.pos.x - resizeButton->getSize().x / 2 + box.size.x / 2;

		if (i < 3)
			pos.y = box.pos.y - resizeButton->getSize().x;
		else if (i > 4)
			pos.y = box.pos.y + box.size.y;
		else
			pos.y = box.pos.y - resizeButton->getSize().x / 2 + box.size.y / 2;
		resizeButton->setPosition("boxes.w / 2 + " + std::to_string(pos.x), "boxes.h / 2 + " + std::to_string(pos.y + 300));
	}
}

void	selectBox(tgui::Button::Ptr button, ShadyCore::Schema::Sequence::BBox *box)
{
	spriteSelected = false;
	selectedBox = box;
	if (boxButton)
		boxButton->getRenderer()->setBackgroundColor(normalColor);
	if (button) {
		normalColor = button->getRenderer()->getBackgroundColor();
		button->getRenderer()->setBackgroundColor(button->getRenderer()->getBackgroundColorFocused());
	}
	boxButton = button;
	for (auto &b : resizeButtons)
		b->setVisible(box != nullptr);
	if (box)
		arrangeButtons(nullptr);
}

void	selectSprite(tgui::Button::Ptr button, std::unique_ptr<EditableObject> &object)
{
	boxButton = button;
	normalColor = button->getRenderer()->getBackgroundColor();
	button->getRenderer()->setBackgroundColor(button->getRenderer()->getBackgroundColorFocused());
	spriteSelected = true;
	for (auto &b : resizeButtons)
		b->setVisible(true);
	arrangeButtons(&*object);
}

void	verifySettings()
{
	if (!soku.empty())
		try {
			if (!std::filesystem::exists(std::filesystem::path(soku) / "th123a.dat"))
				throw std::runtime_error("th123a.dat was not found in " + soku);
			if (!std::filesystem::exists(std::filesystem::path(soku) / "th123b.dat"))
				throw std::runtime_error("th123b.dat was not found in " + soku);
			if (!std::filesystem::exists(std::filesystem::path(soku) / "th123c.dat"))
				throw std::runtime_error("th123c.dat was not found in " + soku);
		} catch (const std::exception &e) {
			soku.clear();
			Utils::dispMsg("Settings loading error", "Loaded soku directory is invalid: " + std::string(e.what()) + ".\nPlease select a valid Soku directory", MB_ICONERROR);
		}

	if (!swr.empty())
		try {
			if (!std::filesystem::exists(std::filesystem::path(swr) / "th105a.dat"))
				throw std::runtime_error("th105a.dat was not found in " + swr);
			if (!std::filesystem::exists(std::filesystem::path(swr) / "th105b.dat"))
				throw std::runtime_error("th105b.dat was not found in " + swr);
		} catch (const std::exception &e) {
			swr.clear();
			Utils::dispMsg("Settings loading error", "Loaded SWT directory is invalid: " + std::string(e.what()) + ".\nPlease select a valid SWR directory", MB_ICONERROR);
		}

	if (soku.empty()) {
		soku = Utils::openFileDialog("Select Soku dat", ".", {{"th123.\\.dat", "Touhou 12.3 dat file"}});
		if (soku.empty())
			throw std::runtime_error("No Soku dat file selected");
		soku = std::filesystem::path(soku).parent_path().string();
	}
	if (!std::filesystem::exists(std::filesystem::path(soku) / "th123a.dat"))
		throw std::runtime_error("th123a.dat was not found in " + soku);
	if (!std::filesystem::exists(std::filesystem::path(soku) / "th123b.dat"))
		throw std::runtime_error("th123b.dat was not found in " + soku);
	if (!std::filesystem::exists(std::filesystem::path(soku) / "th123c.dat"))
		throw std::runtime_error("th123c.dat was not found in " + soku);

	if (
		std::filesystem::exists(std::filesystem::path(soku) / "th105a.dat") &&
		std::filesystem::exists(std::filesystem::path(soku) / "th105b.dat")
	) {
		game->logger.info("Autodetected SWR dats in " + soku);
		swr = soku;
	}

	if (swr.empty()) {
		swr = Utils::openFileDialog("Select SWR dat", ".", {{"th105.\\.dat", "Touhou 10.5 dat file"}});
		if (swr.empty())
			throw std::runtime_error("No SWR dat file selected");
		swr = std::filesystem::path(swr).parent_path().string();
	}
	if (!std::filesystem::exists(std::filesystem::path(swr) / "th105a.dat"))
		throw std::runtime_error("th105a.dat was not found in " + swr);
	if (!std::filesystem::exists(std::filesystem::path(swr) / "th105b.dat"))
		throw std::runtime_error("th105b.dat was not found in " + swr);
}

void loadPackages()
{
	std::list<std::filesystem::path> paths = {
		std::filesystem::path(soku) / "th123c.dat",
		std::filesystem::path(soku) / "th123b.dat",
		std::filesystem::path(soku) / "th123a.dat",
		std::filesystem::path(swr)  / "th105b.dat",
		std::filesystem::path(swr)  / "th105a.dat",
	};

	try {
		for (auto &path: std::filesystem::directory_iterator(soku2))
			paths.push_front(soku2 / path.path().filename() / (path.path().filename().string() + ".dat"));
	} catch (const std::exception &e) {
		game->logger.error("Error loading soku2: " + std::string(e.what()));
	}

	game->package.clear();
	game->characterPaths.clear();
	extraPackages.clear();

	for (auto &path : extra)
		try {
			extraPackages.push_back(new ShadyCore::Package(path));
		} catch (std::exception &e) {
			Utils::dispMsg("Custom package loading error", path + ": " + e.what(), MB_ICONERROR);
		}

	for (auto &path : extraPackages)
		game->package.merge(path);
	for (auto &path : paths)
		game->package.merge(path);

	game->logger.debug("Starting to load packages");
	for (auto &entry : game->package) {
		if (entry.first.name.substr(0, strlen("data_character_")) != "data_character_")
			continue;

		std::string name = std::string(entry.first.name.substr(strlen("data_character_")));
		std::string filename = name.substr(name.find_last_of('_') + 1);

		name = name.substr(0, name.find('_'));
		if (entry.first.fileType == ShadyCore::FileType::TYPE_SCHEMA)
			game->logger.debug("Found schema " + std::string(entry.first.name));
		else if (entry.first.fileType == ShadyCore::FileType::TYPE_PALETTE) {
			auto &pal = game->characterPaths[name].palettes["data/character/" + name + "/" + filename + ".pal"];
			auto &stream = entry.second->open();

			game->logger.debug("Found palette " + std::string(entry.first.name));
			ShadyCore::getResourceReader(entry.first.fileType)(&pal, stream);
			if (pal.bitsPerPixel > 16)
				pal.pack();
			entry.second->close(stream);
		}
	}
	game->logger.debug("Packages are loaded");
}

void	refreshBoxes(tgui::Panel::Ptr panel, FrameData &data, std::unique_ptr<EditableObject> &object)
{
	int i = 0;
	auto button = tgui::Button::create();
	auto renderer = button->getRenderer();
	SpiralOfFate::Vector2f scaleReal{
		(data.frame->blendOptions.scaleX ? data.frame->blendOptions.scaleX : 100) / 100.f,
		(data.frame->blendOptions.scaleY ? data.frame->blendOptions.scaleY : 100) / 100.f
	};

	game->logger.debug("Refresh boxes");
	panel->removeAllWidgets();
	renderer->setBackgroundColor({0xFF, 0xFF, 0xFF, 0x00});
	renderer->setBackgroundColorDown({0xFF, 0xFF, 0xFF, 0xA0});
	renderer->setBackgroundColorHover({0xFF, 0xFF, 0xFF, 0x80});
	renderer->setBackgroundColorDisabled({0xFF, 0xFF, 0xFF, 0x4C});
	renderer->setBackgroundColorFocused({0xFF, 0xFF, 0xFF, 0xA0});
	renderer->setBorderColor({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorDown({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorHover({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorDisabled({0xFF, 0xFF, 0xFF});
	renderer->setBorderColorFocused({0xFF, 0xFF, 0xFF});
	renderer->setBorders(1);
	button->setSize(data.frame->texWidth * (data.frame->blendOptions.scaleX ? data.frame->blendOptions.scaleX : 200) / 100.f, data.frame->texHeight * (data.frame->blendOptions.scaleY ? data.frame->blendOptions.scaleY : 200) / 100.f);
	button->setPosition(
		"&.w / 2 + " + std::to_string(-data.frame->offsetX * scaleReal.x),
		"&.h / 2 + " + std::to_string(-data.frame->offsetY * scaleReal.y + 300)
	);
	button->connect("MousePressed", [&object](std::weak_ptr<tgui::Button> self){
		selectSprite(self.lock(), object);
		canDrag = true;
	}, std::weak_ptr<tgui::Button>(button));
	panel->add(button, "SpriteBox");

	for (auto &box : data.frame->hBoxes) {
		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0x00, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0x00, 0xFF, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0x00, 0xFF, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0x00, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0x00, 0xFF, 0x00, 0xA0});
		renderer->setBorderColor({0x00, 0xFF, 0x00});
		renderer->setBorderColorDown({0x00, 0xFF, 0x00});
		renderer->setBorderColorHover({0x00, 0xFF, 0x00});
		renderer->setBorderColorDisabled({0x00, 0xFF, 0x00});
		renderer->setBorderColorFocused({0x00, 0xFF, 0x00});
		renderer->setBorders(1);
		button->setSize((box.right - box.left) + (box.right == box.left), (box.down - box.up) + (box.down == box.up));
		button->setPosition("&.w / 2 + " + std::to_string(box.left), "&.h / 2 + " + std::to_string(box.up + 300));
		button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "HurtBox" + std::to_string(i));
		i++;
	}
	i = 0;
	for (auto &box : data.frame->aBoxes) {
		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0xFF, 0x00, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0xFF, 0x00, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0xFF, 0x00, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0xFF, 0x00, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0xFF, 0x00, 0x00, 0xA0});
		renderer->setBorderColor({0xFF, 0x00, 0x00});
		renderer->setBorderColorDown({0xFF, 0x00, 0x00});
		renderer->setBorderColorHover({0xFF, 0x00, 0x00});
		renderer->setBorderColorDisabled({0xFF, 0x00, 0x00});
		renderer->setBorderColorFocused({0xFF, 0x00, 0x00});
		renderer->setBorders(1);
		button->setSize((box.right - box.left) + (box.right == box.left), (box.down - box.up) + (box.down == box.up));
		button->setPosition("&.w / 2 + " + std::to_string(box.left), "&.h / 2 + " + std::to_string(box.up + 300));
		button->connect("MousePressed", [&box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "HitBox" + std::to_string(i));
		i++;
	}
	if (!data.frame->cBoxes.empty()) {
		auto &box = data.frame->cBoxes.front();

		button = tgui::Button::create();
		renderer = button->getRenderer();
		renderer->setBackgroundColor({0xFF, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorDown({0xFF, 0xFF, 0x00, 0xA0});
		renderer->setBackgroundColorHover({0xFF, 0xFF, 0x00, 0x80});
		renderer->setBackgroundColorDisabled({0xFF, 0xFF, 0x00, 0x4C});
		renderer->setBackgroundColorFocused({0xFF, 0xFF, 0x00, 0xA0});
		renderer->setBorderColor({0xFF, 0xFF, 0x00});
		renderer->setBorderColorDown({0xFF, 0xFF, 0x00});
		renderer->setBorderColorHover({0xFF, 0xFF, 0x00});
		renderer->setBorderColorDisabled({0xFF, 0xFF, 0x00});
		renderer->setBorderColorFocused({0xFF, 0xFF, 0x00});
		renderer->setBorders(1);
		button->setSize((box.right - box.left) + (box.right == box.left), (box.down - box.up) + (box.down == box.up));
		button->setPosition("&.w / 2 + " + std::to_string(box.left), "&.h / 2 + " + std::to_string(box.up + 300));
		button->connect("MousePressed", [&data, &box](std::weak_ptr<tgui::Button> self){
			selectBox(self.lock(), &box);
			canDrag = true;
		}, std::weak_ptr<tgui::Button>(button));
		panel->add(button, "CollisionBox");
	}
}

template<typename T>
std::string parseActionName(const std::string &name, T wid)
{
	auto r = wid->getRenderer();

	if (name.front() != '$') {
		r->setTextColor(tgui::Color{190, 190, 190});
		return name;
	}

	try {
		tgui::Color color{"#" + name.substr(1, 6)};

		r->setTextColor(color);
		return name.substr(7);
	} catch (...) {
		return name;
	}
}

void	refreshFrameDataPanel(tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<EditableObject> &object)
{
	auto blendMode = panel->get<tgui::EditBox>("BlendMode");
	auto blendColor = panel->get<tgui::EditBox>("BlendColor");
	auto blendColorBut = panel->get<tgui::Button>("BlendColorBut");
	auto scale = panel->get<tgui::EditBox>("Scale");
	auto renderGroup = panel->get<tgui::EditBox>("renderGroup");
	auto blendFlipVert = panel->get<tgui::EditBox>("BlendFlipVert");
	auto blendFlipHorz = panel->get<tgui::EditBox>("BlendFlipHorz");
	auto blendAngle = panel->get<tgui::EditBox>("BlendAngle");
	auto pBlockStun = panel->get<tgui::EditBox>("PBlockStun");
	auto eBlockStun = panel->get<tgui::EditBox>("EBlockStun");
	auto pHitStun = panel->get<tgui::EditBox>("PHitStun");
	auto eHitStun = panel->get<tgui::EditBox>("EHitStun");
	auto hCardGain = panel->get<tgui::EditBox>("HCardGain");
	auto bCardGain = panel->get<tgui::EditBox>("BCardGain");
	auto hitSpeed = panel->get<tgui::EditBox>("HitSpeed");
	auto hitSound = panel->get<tgui::EditBox>("HitSFX");
	auto hitEffect = panel->get<tgui::EditBox>("HitEffect");
	auto damage = panel->get<tgui::EditBox>("Damage");
	auto spiritDamage = panel->get<tgui::EditBox>("SpiritDmg");
	auto power = panel->get<tgui::EditBox>("Power");
	auto chip = panel->get<tgui::EditBox>("ChipDmg");
	auto limit = panel->get<tgui::EditBox>("Limit");
	auto speed = panel->get<tgui::EditBox>("MoveSpeed");
	auto aHitSequ = panel->get<tgui::EditBox>("AHitSequ");
	auto gHitSequ = panel->get<tgui::EditBox>("GHitSequ");
	auto prorate = panel->get<tgui::EditBox>("Rate");
	auto untech = panel->get<tgui::EditBox>("Untech");
	auto atkLvl = panel->get<tgui::ComboBox>("AtkLvl");
	auto pos = panel->get<tgui::EditBox>("Pos");
	auto posExtra = panel->get<tgui::EditBox>("PosExtra");
	auto pivot = panel->get<tgui::EditBox>("Pivot");
	auto loop = panel->get<tgui::CheckBox>("Loop");
	auto aCancel = panel->get<tgui::EditBox>("ACancel");
	auto mCancel = panel->get<tgui::EditBox>("MCancel");

	auto mods = panel->get<tgui::EditBox>("Modifiers");
	auto progress = panel->get<tgui::Slider>("Progress");
	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto offset = panel->get<tgui::EditBox>("Offset");
	auto bounds = panel->get<tgui::EditBox>("Bounds");
	auto duration = panel->get<tgui::EditBox>("Duration");
	auto collisionBox = panel->get<tgui::CheckBox>("Collision");
	auto fFlags = panel->get<tgui::EditBox>("fFlags");
	auto aFlags = panel->get<tgui::EditBox>("aFlags");
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
	auto actionName = panel->get<tgui::Button>("ActionName");
	auto name = actionNames.find(object->_action);
	auto renderer = blendColorBut->getRenderer();
	char buffer[10];
	sf::Color color = SokuColor(data.frame->blendOptions.color);

	sprintf(buffer, "#%08X", data.frame->blendOptions.color);
	game->logger.debug("Soft refresh");
	*c = true;
	actionName->setText(parseActionName(name == actionNames.end() ? "Action #" + std::to_string(object->_action) : name->second, actionName));
	fFlags->setText(std::to_string(data.frame->traits.frameFlags));
	aFlags->setText(std::to_string(data.frame->traits.attackFlags));
	mods->setText(std::to_string(data.frame->traits.comboModifier));
	chip->setText(std::to_string(data.frame->traits.chipDamage));
	progress->setMinimum(0);
	progress->setMaximum(object->_moves.at(object->_action)[object->_actionBlock].size() - 1);
	progress->setValue(object->_animation);
	blendMode->setText(std::to_string(data.frame->blendOptions.mode));
	blendColor->setText(buffer);
	renderer->setBackgroundColor(color);
	renderer->setBackgroundColorDisabled(color);
	renderer->setBackgroundColorHover(color);
	renderer->setBackgroundColorDown(color);
	renderer->setBackgroundColorFocused(color);
	blendMode->setText(std::to_string(data.frame->blendOptions.mode));
	renderGroup->setText(std::to_string(data.frame->renderGroup));
	blendFlipVert->setText(std::to_string(data.frame->blendOptions.flipVert));
	blendFlipHorz->setText(std::to_string(data.frame->blendOptions.flipHorz));
	blendAngle->setText(std::to_string(data.frame->blendOptions.angle));
	hCardGain->setText(std::to_string(data.frame->traits.onHitCardGain));
	bCardGain->setText(std::to_string(data.frame->traits.onBlockCardGain));
	hitEffect->setText(std::to_string(data.frame->traits.onHitEffect));
	aHitSequ->setText(std::to_string(data.frame->traits.onAirHitSetSequence));
	gHitSequ->setText(std::to_string(data.frame->traits.onGroundHitSetSequence));
	untech->setText(std::to_string(data.frame->traits.untech));
	atkLvl->setSelectedItemByIndex(data.frame->traits.attackLevel);
	sprite->setText(data._schema->images[data.frame->imageIndex].name);
	hitSound->setText(std::to_string(data.frame->traits.onHitSfx));
	damage->setText(std::to_string(data.frame->traits.damage));
	spiritDamage->setText(std::to_string(data.frame->traits.spiritDamage));
	power->setText(std::to_string(data.frame->traits.power));
	duration->setText(std::to_string(data.frame->duration));
	pBlockStun->setText(std::to_string(data.frame->traits.onBlockPlayerStun));
	eBlockStun->setText(std::to_string(data.frame->traits.onBlockEnemyStun));
	pHitStun->setText(std::to_string(data.frame->traits.onHitPlayerStun));
	eHitStun->setText(std::to_string(data.frame->traits.onHitEnemyStun));
	prorate->setText(std::to_string(data.frame->traits.proration / 10) + "." + std::to_string(data.frame->traits.proration % 10) + "%");
	limit->setText(std::to_string(data.frame->traits.limit));
	loop->setChecked(data.parent->loop);
	aCancel->setText(std::to_string(data.parent->actionLock));
	mCancel->setText(std::to_string(data.parent->moveLock));

	auto newBounds = "(" + std::to_string(data.frame->texOffsetX) + "," + std::to_string(data.frame->texOffsetY) + "," + std::to_string(data.frame->texWidth) + "," + std::to_string(data.frame->texHeight) + ")";
	auto newScale = "(" +
		std::to_string(data.frame->blendOptions.scaleX / 100) + "." + (data.frame->blendOptions.scaleX % 100 < 10 ? "0" : "") + std::to_string(data.frame->blendOptions.scaleX % 100) + "," +
		std::to_string(data.frame->blendOptions.scaleY / 100) + "." + (data.frame->blendOptions.scaleY % 100 < 10 ? "0" : "") + std::to_string(data.frame->blendOptions.scaleY % 100) +
	")";
	auto newOffset = "(" + std::to_string(data.frame->offsetX) + "," + std::to_string(data.frame->offsetY) + ")";
	auto newSpeed = "(" +
		std::to_string(data.frame->effect.speedX / 100) + "." + (data.frame->effect.speedX % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.speedX % 100) + "," +
		std::to_string(data.frame->effect.speedY / 100) + "." + (data.frame->effect.speedY % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.speedY % 100) +
	")";
	auto newHitSpeed = "(" +
		std::to_string(data.frame->traits.speedX / 100) + "." + (data.frame->traits.speedX % 100 < 10 ? "0" : "") + std::to_string(data.frame->traits.speedX % 100) + "," +
		std::to_string(data.frame->traits.speedY / 100) + "." + (data.frame->traits.speedY % 100 < 10 ? "0" : "") + std::to_string(data.frame->traits.speedY % 100) +
	")";
	auto newPos = "(" +
		std::to_string(data.frame->effect.positionXExtra / 100) + "." + (data.frame->effect.positionXExtra % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.positionXExtra % 100) + "," +
		std::to_string(data.frame->effect.positionYExtra / 100) + "." + (data.frame->effect.positionYExtra % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.positionYExtra % 100) +
	")";
	auto newPosExtra = "(" +
		std::to_string(data.frame->effect.positionX / 100) + "." + (data.frame->effect.positionX % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.positionX % 100) + "," +
		std::to_string(data.frame->effect.positionY / 100) + "." + (data.frame->effect.positionY % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.positionY % 100) +
	")";
	auto newPivot = "(" +
		std::to_string(data.frame->effect.pivotX / 100) + "." + (data.frame->effect.pivotX % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.pivotX % 100) + "," +
		std::to_string(data.frame->effect.pivotY / 100) + "." + (data.frame->effect.pivotY % 100 < 10 ? "0" : "") + std::to_string(data.frame->effect.pivotY % 100) +
	")";

	pos->setText(newPos);
	posExtra->setText(newPosExtra);
	pivot->setText(newPivot);
	hitSpeed->setText(newHitSpeed);
	speed->setText(newSpeed);
	offset->setText(newOffset);
	bounds->setText(newBounds);
	scale->setText(newScale);
	collisionBox->setChecked(!data.frame->cBoxes.empty());
	selectBox(nullptr, nullptr);
	refreshBoxes(boxes, data, object);
	*c = false;
}

void createAction(std::unique_ptr<EditableObject> &object, unsigned newAction)
{
	auto &action = object->_moves.at(object->_action);
	auto &data = action[object->_actionBlock][object->_animation];

	tempSchema.objects.reserve(tempSchema.objects.size() + action.size());
	object->_moves[newAction] = {};

	auto it = tempSchema.objects.begin();

	for (;; ++it) {
		if ((*it)->getType() != 9)
			continue;
		if (reinterpret_cast<ShadyCore::Schema::Sequence *>(*it)->getId() > newAction)
			break;
	}
	for (auto &block : action) {
		auto newSequence = new ShadyCore::Schema::Sequence(newAction, false);

		newSequence->moveLock = block.front().parent->moveLock;
		newSequence->actionLock = block.front().parent->actionLock;
		newSequence->loop = block.front().parent->loop;
		newSequence->frames.reserve(block.size());
		tempSchema.objects.insert(it, newSequence);
		object->_moves[newAction].emplace_back();
		it++;
		for (auto &frame : block) {
			auto newElem = new ShadyCore::Schema::Sequence::MoveFrame(*frame.frame);

			newSequence->frames.push_back(newElem);
			object->_moves[newAction].back().emplace_back(editSession.chr, tempSchema, *newSequence, *newElem, *editSession.palette, editSession.palName);
		}
	}

}

void	refreshRightPanel(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, bool resetAction = true)
{
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");
	auto action = panel->get<tgui::EditBox>("Action");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");
	auto frame = panel->get<tgui::SpinButton>("Frame");

	game->logger.debug("Hard refresh");
	panel->setEnabled(static_cast<bool>(object));
	if (!object)
		return gui.get<tgui::Panel>("Boxes")->removeAllWidgets();
	step->onPress.emit(&*step, "Step");
	if (resetAction) {
		auto txt = std::to_string(object->_moves.begin()->first);

		action->setText(txt);
		action->onReturnKeyPress.emit(&*action, txt);
	}
	if (speedCtrl->getValue() == 0)
		speedCtrl->onValueChange.emit(&*speedCtrl, 0);
	else
		speedCtrl->setValue(0);
	//if (object->_moves[0].empty()) {
	//	object->_moves[0].emplace_back();

	//	//TODO: Properly init a default one
	//	auto newSequence = new ShadyCore::Schema::Sequence(0, true);
	//	auto newElem = new ShadyCore::Schema::Sequence::MoveFrame();

	//	newSequence->frames.push_back(newElem);
	//	editSession.schema->objects.push_back(newSequence);
	//	object->_moves[0][0].emplace_back(editSession.chr, *editSession.schema, *newSequence, *newElem, *editSession.palette, editSession.palName);
	//}
	refreshFrameDataPanel(panel, gui.get<tgui::Panel>("Boxes"), object);
}

void	placeAnimPanelHooks(tgui::Gui &gui, tgui::Panel::Ptr panel, tgui::Panel::Ptr boxes, std::unique_ptr<EditableObject> &object)
{
	// Ctrl UI
	auto panWeak = std::weak_ptr<tgui::Panel>(panel);
	auto animPanel = panel->get<tgui::Panel>("AnimPanel");
	auto action = panel->get<tgui::EditBox>("Action");
	auto actionName = panel->get<tgui::Button>("ActionName");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto blockLabel = panel->get<tgui::Label>("Label1");
	auto frameLabel = panel->get<tgui::Label>("Label2");
	auto progress = panel->get<tgui::Slider>("Progress");
	auto play = panel->get<tgui::Button>("Play");
	auto step = panel->get<tgui::Button>("Step");
	auto speedCtrl = panel->get<tgui::SpinButton>("Speed");
	auto speedLabel = panel->get<tgui::Label>("SpeedLabel");

	auto sprite = panel->get<tgui::EditBox>("Sprite");
	auto hitSound = panel->get<tgui::EditBox>("HitSFX");
	auto offset = panel->get<tgui::EditBox>("Offset");
	auto bounds = panel->get<tgui::EditBox>("Bounds");
	auto scale = panel->get<tgui::EditBox>("Scale");
	auto collisionBox = panel->get<tgui::CheckBox>("Collision");
	auto duration = panel->get<tgui::EditBox>("Duration");
	auto pBlockStun = panel->get<tgui::EditBox>("PBlockStun");
	auto eBlockStun = panel->get<tgui::EditBox>("EBlockStun");
	auto pHitStun = panel->get<tgui::EditBox>("PHitStun");
	auto eHitStun = panel->get<tgui::EditBox>("EHitStun");
	auto prorate = panel->get<tgui::EditBox>("Rate");
	auto damage = panel->get<tgui::EditBox>("Damage");
	auto spiritDamage = panel->get<tgui::EditBox>("SpiritDmg");
	auto power = panel->get<tgui::EditBox>("Power");
	auto hitSpeed = panel->get<tgui::EditBox>("HitSpeed");
	auto limit = panel->get<tgui::EditBox>("Limit");
	auto gravity = panel->get<tgui::EditBox>("Gravity");
	auto speed = panel->get<tgui::EditBox>("MoveSpeed");
	auto aFlags = panel->get<tgui::EditBox>("aFlags");
	auto fFlags = panel->get<tgui::EditBox>("fFlags");
	auto mods = panel->get<tgui::EditBox>("Modifiers");
	auto chip = panel->get<tgui::EditBox>("ChipDmg");
	auto loop = panel->get<tgui::CheckBox>("Loop");
	auto aCancel = panel->get<tgui::EditBox>("ACancel");
	auto mCancel = panel->get<tgui::EditBox>("MCancel");

	auto blendMode = panel->get<tgui::EditBox>("BlendMode");
	auto blendColor = panel->get<tgui::EditBox>("BlendColor");
	auto blendColorBut = panel->get<tgui::Button>("BlendColorBut");
	auto renderGroup = panel->get<tgui::EditBox>("renderGroup");
	auto blendFlipVert = panel->get<tgui::EditBox>("BlendFlipVert");
	auto blendFlipHorz = panel->get<tgui::EditBox>("BlendFlipHorz");
	auto blendAngle = panel->get<tgui::EditBox>("BlendAngle");
	auto hCardGain = panel->get<tgui::EditBox>("HCardGain");
	auto bCardGain = panel->get<tgui::EditBox>("BCardGain");
	auto hitEffect = panel->get<tgui::EditBox>("HitEffect");
	auto aHitSequ = panel->get<tgui::EditBox>("AHitSequ");
	auto gHitSequ = panel->get<tgui::EditBox>("GHitSequ");
	auto untech = panel->get<tgui::EditBox>("Untech");
	auto atkLvl = panel->get<tgui::ComboBox>("AtkLvl");
	auto pos = panel->get<tgui::EditBox>("Pos");
	auto posExtra = panel->get<tgui::EditBox>("PosExtra");
	auto pivot = panel->get<tgui::EditBox>("Pivot");
	auto renderer = blendColorBut->getRenderer();

	std::vector<tgui::CheckBox::Ptr> fFlagsCheckboxes;
	std::vector<tgui::CheckBox::Ptr> aFlagsCheckboxes;
	std::vector<tgui::CheckBox::Ptr> modsCheckboxes;

	actionName->connect("Clicked", [&gui, &object, block, action]{
		auto window = Utils::openWindowWithFocus(gui, 500, "&.h - 100");
		auto pan = tgui::ScrollablePanel::create({"&.w", "&.h"});
		unsigned i = 0;
		auto scroll = 0;
		std::vector<unsigned> toDisplay;

		for (auto &a : actionNames)
			toDisplay.push_back(a.first);
		for (auto &a : object->_moves) {
			if (std::find(toDisplay.begin(), toDisplay.end(), a.first) == toDisplay.end())
				toDisplay.push_back(a.first);
		}
		std::sort(toDisplay.begin(), toDisplay.end());
		Utils::setRenderer(pan);
		window->setTitle("Default character moves");
		window->add(pan);
		for (auto &moveId : toDisplay) {
			auto it = actionNames.find(moveId);
			auto label = tgui::Label::create(std::to_string(moveId));
			auto button = tgui::Button::create();

			label->setPosition(10, i * 25 + 12);
			button->setPosition(50, i * 25 + 10);
			button->setSize(430, 20);
			Utils::setRenderer(button);
			Utils::setRenderer(label);
			button->setText(parseActionName(it == actionNames.end() ? "Unnamed move #" + std::to_string(moveId) : it->second, button));
			if (moveId == object->_action) {
				if (i > 30)
					scroll = (i - 20) * 25;
				button->getRenderer()->setTextColor(tgui::Color::Green);
				button->getRenderer()->setTextColorHover(tgui::Color{0x40, 0xFF, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0x00, 0xA0, 0x00});
				button->getRenderer()->setTextColorDown(tgui::Color{0x00, 0x80, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x20, 0x80, 0x20});
			} else if (object->_moves.find(moveId) == object->_moves.end()) {
				button->getRenderer()->setTextColor(tgui::Color::Red);
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0x40, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x00, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x20, 0x20});
			} else if (it == actionNames.end()) {
				button->getRenderer()->setTextColor(tgui::Color{0xFF, 0x80, 0x00});
				button->getRenderer()->setTextColorHover(tgui::Color{0xFF, 0xA0, 0x40});
				button->getRenderer()->setTextColorDisabled(tgui::Color{0xA0, 0x50, 0});
				button->getRenderer()->setTextColorDown(tgui::Color{0x80, 0x40, 0x00});
				button->getRenderer()->setTextColorFocused(tgui::Color{0x80, 0x60, 0x20});
			}

			button->connect("Clicked", [moveId, action](std::weak_ptr<tgui::ChildWindow> win){
				win.lock()->close();
				action->setText(std::to_string(moveId));
				action->onReturnKeyPress.emit(&*action, action->getText());
			}, std::weak_ptr<tgui::ChildWindow>(window));

			pan->add(label);
			pan->add(button);
			i++;
		}
		pan->setVerticalScrollbarValue(scroll);

		auto label = tgui::Label::create("");

		label->setPosition(10, i * 25 + 10);
		label->setSize(10, 5);
		pan->add(label);
	});
	boxes->connect("Clicked", []{
		if (!dragStart && !canDrag)
			selectBox(nullptr, nullptr);
	});
	action->connect("ReturnKeyPressed", [&object, block](const std::string &t){
		if (t.empty())
			return;
		auto newAction = std::stoul(t);
		if (object->_moves.find(newAction) == object->_moves.end())
			createAction(object, newAction);
		object->_action = newAction;
		block->setMaximum(object->_moves[object->_action].size() - 1);
		block->setMinimum(0);
		if (block->getValue() == 0)
			block->onValueChange.emit(&*block, 0);
		else
			block->setValue(0);
	});
	block->connect("ValueChanged", [&object, blockLabel, progress](float f){
		int i = f;

		blockLabel->setText("Block " + std::to_string(i + 1) + "/" + std::to_string(object->_moves[object->_action].size()));
		object->_actionBlock = i;
		/*if (object->_moves[object->_action][i].empty()) {
			game->logger.warn("This block is empty so we generate a frame");
			auto newSequence = new ShadyCore::Schema::Sequence(object->_action, false);
			auto newElem = new ShadyCore::Schema::Sequence::MoveFrame();

			//TODO: Init a proper value
			newSequence->moveLock = 0;
			newSequence->actionLock = 0;
			newSequence->loop = false;
			newSequence->frames.push_back(newElem);
			tempSchema.objects.push_back(newSequence);
			object->_moves[object->_action][i].emplace_back(editSession.chr, tempSchema, *newSequence, *newElem, *editSession.palette, editSession.palName);
		}*/
		progress->setMaximum(object->_moves[object->_action][i].size() - 1);
		progress->setMinimum(0);
		if (progress->getValue() == 0)
			progress->onValueChange.emit(&*progress, 0);
		else
			progress->setValue(0);
	});
	progress->connect("ValueChanged", [boxes, &object, frameLabel, panWeak](float f){
		int i = f;

		frameLabel->setText("Frame " + std::to_string(i));
		object->_animation = i;
		object->_animationCtr = 0;
		refreshFrameDataPanel(panWeak.lock(), boxes, object);
	});
	play->connect("Clicked", [speedLabel, speedCtrl]{
		speedCtrl->setValue(1);
	});
	step->connect("Clicked", [speedLabel, speedCtrl]{
		updateAnyway = true;
		speedCtrl->setValue(0);
	});
	speedCtrl->connect("ValueChanged", [speedLabel, animPanel](float f){
		speedLabel->setText(f == 0 ? "Paused" : "x" + std::to_string(f));
		updateTimer = 1 / f;
		timer = 0;
		animPanel->setEnabled(f == 0);
	});
	chip->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.chipDamage = std::stoul(t);
	});
	blendMode->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->blendOptions.mode = std::stoul(t);
	});
	blendColor->connect("TextChanged", [&object, renderer](std::string t){
		if (*c || t.size() != 9)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->blendOptions.color = std::stoul(t.substr(1), nullptr, 16);

		sf::Color color = SokuColor(data.frame->blendOptions.color);

		renderer->setBackgroundColor(color);
		renderer->setBackgroundColorDisabled(color);
		renderer->setBackgroundColorHover(color);
		renderer->setBackgroundColorDown(color);
		renderer->setBackgroundColorFocused(color);
	});
	blendColorBut->connect("Clicked", [&gui, &object, blendColor](){
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		sf::Color color = SokuColor(data.frame->blendOptions.color);

		Utils::makeColorPickWindow(gui, [blendColor](sf::Color col){
			char buffer[10];

			sprintf(buffer, "#%08X", SokuColor(col.r, col.g, col.b, col.a).color);
			blendColor->setText(buffer);
		}, color);
	});
	renderGroup->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->renderGroup = std::stoul(t);
	});
	blendFlipVert->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->blendOptions.flipVert = std::stoul(t);
	});
	blendFlipHorz->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->blendOptions.flipHorz = std::stoul(t);
	});
	blendAngle->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			data.frame->blendOptions.angle = std::stoi(t);
		} catch (...) {}
	});
	hCardGain->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onHitCardGain = std::stoul(t);
	});
	bCardGain->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onBlockCardGain = std::stoul(t);
	});
	hitEffect->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onHitEffect = std::stoul(t);
	});
	aHitSequ->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onAirHitSetSequence = std::stoul(t);
	});
	gHitSequ->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onGroundHitSetSequence = std::stoul(t);
	});
	untech->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.untech = std::stoul(t);
	});
	atkLvl->connect("ItemSelected", [&object](int i){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.attackLevel = i;
	});
	pos->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.frame->effect.positionX = std::stof(x) * 100;
			data.frame->effect.positionY = std::stof(y) * 100;
		} catch (std::exception &) {}
	});
	posExtra->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.frame->effect.positionXExtra = std::stof(x) * 100;
			data.frame->effect.positionYExtra = std::stof(y) * 100;
		} catch (std::exception &) {}
	});
	pivot->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.frame->effect.pivotX = std::stof(x) * 100;
			data.frame->effect.pivotY = std::stof(y) * 100;
		} catch (std::exception &) {}
	});
	sprite->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->imageIndex = getOrCreateImage(tempSchema, t);
		data.reloadTexture();
	});
	hitSound->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onHitSfx = std::stoul(t);
	});
	speed->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.frame->effect.speedX = std::stof(x) * 100;
			data.frame->effect.speedY = std::stof(y) * 100;
			refreshBoxes(boxes, data, object);
			if (spriteSelected)
				arrangeButtons(&*object);
		} catch (std::exception &) {}
	});
	offset->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stol(y);
			data.frame->offsetX = std::stol(x);
			data.frame->offsetY = std::stol(y);
			refreshBoxes(boxes, data, object);
			if (spriteSelected)
				arrangeButtons(&*object);
		} catch (std::exception &) {}
	});
	hitSpeed->connect("TextChanged", [boxes, &object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			data.frame->traits.speedX = std::stof(x) * 100;
			data.frame->traits.speedY = std::stof(y) * 100;
		} catch (...) {}
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
	});
	scale->connect("TextChanged", [&object, boxes](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);
		auto y = t.substr(pos + 1, t.size() - pos - 1);
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stoul(y);
			data.frame->blendOptions.scaleX = std::stof(x) * 100;
			data.frame->blendOptions.scaleY = std::stof(y) * 100;
		} catch (...) {}
		refreshBoxes(boxes, data, object);
		if (spriteSelected)
			arrangeButtons(&*object);
	});
	bounds->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);

		auto remainder = t.substr(pos + 1);
		auto pos2 = remainder.find(',');
		auto y = remainder.substr(0, pos2 + 1);

		auto remainder2 = remainder.substr(pos2 + 1);
		auto pos3 = remainder2.find(',');
		auto w = remainder2.substr(0, pos3 + 1);
		auto h = remainder2.substr(pos3 + 1, remainder2.size() - pos3 - 1);;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		try {
			std::stoul(x);
			std::stoul(y);
			std::stoul(w);
			data.frame->texOffsetX = std::stoul(x);
			data.frame->texOffsetY = std::stoul(y);
			data.frame->texWidth   = std::stoul(w);
			data.frame->texHeight  = std::stoul(h);
		} catch (...) {}
	});
	duration->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->duration = std::stoul(t);
	});
	damage->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.damage = std::stoul(t);
	});
	spiritDamage->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.spiritDamage = std::stoul(t);
	});
	power->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.power = std::stoul(t);
	});
	pBlockStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onBlockPlayerStun = std::stoul(t);
	});
	eBlockStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onBlockEnemyStun = std::stoul(t);
	});
	pHitStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onHitPlayerStun = std::stoul(t);
	});
	eHitStun->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.onHitEnemyStun = std::stoul(t);
	});
	limit->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.limit = std::stoul(t);
	});
	prorate->connect("TextChanged", [&object](std::string t){
		if (*c)
			return;
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.frame->traits.proration = std::stof(t) * 10;
	});
	collisionBox->connect("Checked", [&object, boxes]{
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		if (data.frame->cBoxes.empty())
			data.frame->cBoxes.push_back({
				static_cast<int>(-20),
				static_cast<int>(-200),
				static_cast<int>(20),
				static_cast<int>(0),
			});
		refreshBoxes(boxes, data, object);
	});
	collisionBox->connect("Unchecked", [&object, boxes]{
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		if (selectedBox && selectedBox == &data.frame->cBoxes.front())
			selectBox(nullptr, nullptr);
		data.frame->cBoxes.clear();
		refreshBoxes(boxes, data, object);
	});
	loop->connect("Changed", [&object](bool b){
		if (*c)
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.parent->loop = b;
	});
	aCancel->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.parent->actionLock = std::stoul(t);
	});
	mCancel->connect("TextChanged", [&object](std::string t){
		if (*c || t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		data.parent->moveLock = std::stoul(t);
	});

	for (int i = 0; i < 32; i++) {
		auto checkbox = panel->get<tgui::CheckBox>("fFlag" + std::to_string(i));

		fFlagsCheckboxes.push_back(checkbox);
		if (!checkbox)
			continue;
		checkbox->connect("Changed", [i, fFlags, &object](bool b){
			if (*c)
				return;

			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			*c = true;
			data.frame->traits.frameFlags = (data.frame->traits.frameFlags & ~(1 << i)) | b << i;
			fFlags->setText(std::to_string(data.frame->traits.frameFlags));
			*c = false;
		});
	}
	fFlags->connect("TextChanged", [&object, fFlagsCheckboxes](std::string t){
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		auto g = *c;

		if (!*c) {
			*c = true;
			data.frame->traits.frameFlags = std::stoul(t);
		}
		for (int i = 0; i < 32; i++)
			if (fFlagsCheckboxes[i])
				fFlagsCheckboxes[i]->setChecked((data.frame->traits.frameFlags & (1 << i)) != 0);
		if (!g)
			*c = false;
	});

	for (int i = 0; i < 32; i++) {
		auto checkbox = panel->get<tgui::CheckBox>("aFlag" + std::to_string(i));

		aFlagsCheckboxes.push_back(checkbox);
		if (!checkbox)
			continue;
		checkbox->connect("Changed", [i, aFlags, &object](bool b){
			if (*c)
				return;

			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			*c = true;
			data.frame->traits.attackFlags = (data.frame->traits.attackFlags & ~(1 << i)) | b << i;
			aFlags->setText(std::to_string(data.frame->traits.attackFlags));
			*c = false;
		});
	}
	aFlags->connect("TextChanged", [&object, aFlagsCheckboxes](std::string t){
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		auto g = *c;

		if (!*c) {
			*c = true;
			data.frame->traits.attackFlags = std::stoul(t);
		}
		for (int i = 0; i < 32; i++)
			if (aFlagsCheckboxes[i])
				aFlagsCheckboxes[i]->setChecked((data.frame->traits.attackFlags & (1 << i)) != 0);
		if (!g)
			*c = false;
	});

	for (int i = 0; i < 8; i++) {
		auto checkbox = panel->get<tgui::CheckBox>("Mod" + std::to_string(i));

		modsCheckboxes.push_back(checkbox);
		if (!checkbox)
			continue;
		checkbox->connect("Changed", [i, aFlags, &object](bool b){
			if (*c)
				return;

			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			*c = true;
			data.frame->traits.comboModifier = (data.frame->traits.comboModifier & ~(1 << i)) | b << i;
			aFlags->setText(std::to_string(data.frame->traits.comboModifier));
			*c = false;
		});
	}
	mods->connect("TextChanged", [&object, modsCheckboxes](std::string t){
		if (t.empty())
			return;

		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
		auto g = *c;

		if (!*c) {
			*c = true;
			data.frame->traits.comboModifier = std::stoul(t);
		}
		for (int i = 0; i < 8; i++)
			if (modsCheckboxes[i])
				modsCheckboxes[i]->setChecked((data.frame->traits.comboModifier & (1 << i)) != 0);
		if (!g)
			*c = false;
	});
}

void	removeBoxCallback(tgui::Panel::Ptr boxes, std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr panel)
{
	if (!selectedBox)
		return;

	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	if (selectedBox == &data.frame->cBoxes.front()) {
		auto block = panel->get<tgui::CheckBox>("Collision");

		block->setChecked(false);
		selectBox(nullptr, nullptr);
		return;
	}
	for (auto it = data.frame->aBoxes.begin(); it < data.frame->aBoxes.end(); it++) {
		if (&*it == selectedBox) {
			data.frame->aBoxes.erase(it);
			refreshBoxes(boxes, data, object);
			selectBox(nullptr, nullptr);
			return;
		}
	}
	for (auto it = data.frame->hBoxes.begin(); it < data.frame->hBoxes.end(); it++) {
		if (&*it == selectedBox) {
			data.frame->hBoxes.erase(it);
			refreshBoxes(boxes, data, object);
			selectBox(nullptr, nullptr);
			return;
		}
	}
	my_assert(false);
}

void	quitCallback()
{
	quitRequest = true;
}

void	newFileCallback(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	Utils::dispMsg("Error", "Not implemented yet", MB_ICONERROR);
	return;
	object = std::make_unique<EditableObject>();
	overlayData.object = std::make_unique<OverlayObject>(object->_moves);
	overlayData.object->displayScaled = true;
	loadedPath.clear();
	object->_moves[0].emplace_back();
	refreshRightPanel(gui, object);
	bar->setMenuEnabled({"New"}, true);
	bar->setMenuEnabled({"Remove"}, true);
	bar->setMenuEnabled({"Misc"}, true);
	bar->setMenuItemEnabled({"File", "Export..."}, true);
	bar->setMenuItemEnabled({"File", "Import...", "palette..."}, true);
}

bool loadLabel(const std::string &path)
{
	std::ifstream stream{path};
	nlohmann::json json;

	if (!stream) {
		game->logger.error("Failed loading " + path + ": " + strerror(errno));
		return false;
	}
	stream >> json;
	for (auto &j : json.items()) {
		unsigned long id = std::stoul(j.key());

		if (!j.value().is_null())
			actionNames[id] = j.value();
		else if (actionNames.find(id) != actionNames.end())
			actionNames.erase(actionNames.find(id));
	}
	return true;
}

void loadLabels(const std::string &character)
{
	actionNames.clear();
	try {
		if (!loadLabel("assets/labels/common.json"))
			actionNames = defaultActionNames;
		loadLabel("assets/labels/" + character + ".json");
	} catch (const std::exception &e) {
		game->logger.error("Error loading labels for " + character + ": " + e.what());
		Utils::dispMsg("Error", "Error loading labels: " + std::string(e.what()), MB_ICONERROR);
		actionNames = defaultActionNames;
	}
}

void openFramedataFromPackage(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	auto window = Utils::openWindowWithFocus(gui, 300, 300);
	std::vector<std::string> chrs;
	tgui::ListBox::Ptr list = tgui::ListBox::create();

	for (auto &chr : game->characterPaths)
		chrs.push_back(chr.first);

	window->setTitle("Select character's framedata to load");
	std::sort(chrs.begin(), chrs.end());
	list->connect("DoubleClicked", [&object, bar, &gui, window](const std::string &character){
		editSession.palName = "data/character/" + character + "/palette000.pal";

		//auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});
		std::string path = "data/character/" + character + "/" + character + ".pat";
		auto pal = game->characterPaths[character].palettes.find(editSession.palName);
		auto entry = game->package.find(path, ShadyCore::FileType::TYPE_SCHEMA);

		game->logger.info("Loading framedata from package: " + path);
		if (entry == game->package.end()) {
			Utils::dispMsg("Loading error", "Could not find file " + path, MB_ICONERROR);
			return;
		}
		if (pal == game->characterPaths[character].palettes.end()) {
			Utils::dispMsg("Loading error", "Could not find palette " + editSession.palName, MB_ICONERROR);
			return;
		}

		auto &stream = entry.open();

		editSession.chr = character;
		editSession.palName = pal->first;
		editSession.palette = &pal->second;
		tempSchema.destroy();
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_SCHEMA, ShadyCore::FileType::SCHEMA_GAME_PATTERN})(&tempSchema, stream);
		entry.close(stream);
		try {
			object.reset();
			object = std::make_unique<EditableObject>(editSession.chr, tempSchema, *editSession.palette, editSession.palName);
			overlayData.object = std::make_unique<OverlayObject>(object->_moves);
			overlayData.object->displayScaled = true;
			loadedPath = path;
			loadLabels(character);
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, true);
			bar->setMenuEnabled({"Remove"}, true);
			bar->setMenuEnabled({"Misc"}, true);
			bar->setMenuItemEnabled({"File", "Export..."}, true);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, true);
		} catch (std::exception &e) {
			Utils::dispMsg("Error", e.what(), MB_ICONERROR);
			loadedPath = path;
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, false);
			bar->setMenuEnabled({"Remove"}, false);
			bar->setMenuEnabled({"Misc"}, false);
			bar->setMenuItemEnabled({"File", "Export..."}, false);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);
		}
		displayPalEditor(object, gui);
		window->close();
	});
	for (auto &character : chrs)
		list->addItem(character);
	list->setSize("&.w", "&.h");
	list->setScrollbarValue(0);
	window->add(list);
}

bool saveFramedataToFileCallback(std::unique_ptr<EditableObject> &)
{
	auto path = Utils::saveFileDialog(
		"Save pattern file",
		editSession.chr,
		{{".*\\.pat", "Pattern file"}, {".*\\.xml", "XML file"}}
	);

	if (path.empty())
		return false;

	std::string ext = path.substr(path.find_last_of('.') + 1);
	std::ofstream stream{path, std::fstream::binary};

	if (stream.fail()) {
		Utils::dispMsg("Error", "Could not open file " + path, MB_ICONERROR);
		return false;
	}
	ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_SCHEMA, ext == "xml" ? ShadyCore::FileType::SCHEMA_XML : ShadyCore::FileType::SCHEMA_GAME_PATTERN})(&tempSchema, stream);
	return true;
}

void openFramedataFromFile(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	auto p = Utils::openFileDialog("Open framedata file", ".",{{".*\\.pat", "Pattern file"}, {".*\\.xml", "XML file"}});

	if (p.empty())
		return;

	auto fct = [bar, &gui, &object, p]{
		auto win = Utils::openWindowWithFocus(gui, 250, 70);
		std::string path = p;

		win->loadWidgetsFromFile("assets/gui/edit.gui");

		auto edit = win->get<tgui::EditBox>("EditBox");
		auto cancel = win->get<tgui::Button>("Cancel");
		auto ok = win->get<tgui::Button>("OK");
		size_t pos;

		try {
			while ((pos = path.find_last_of('\\')) != std::string::npos)
				path[pos] = '/';
			edit->setText(path.substr(path.find_last_of('/') + 1, path.find_last_of('.') - path.find_last_of('/') - 1));
		} catch (...) {}
		win->setTitle("Enter character name");
		ok->connect("Clicked", [win, edit, &object, bar, &gui, path]{
			std::string character = edit->getText();

			editSession.palName = "data/character/" + character + "/palette000.pal";

			//auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});
			auto pal = game->characterPaths[character].palettes.find(editSession.palName);
			std::ifstream stream{path, std::fstream::binary};
			std::string ext = std::filesystem::path(path).extension().string();

			game->logger.info("Loading framedata from file: " + path);
			if (stream.fail()) {
				Utils::dispMsg("Loading error", "Could not find file " + path + ": " + strerror(errno), MB_ICONERROR);
				return;
			}
			if (pal == game->characterPaths[character].palettes.end()) {
				Utils::dispMsg("Loading error", "Could not find palette " + editSession.palName, MB_ICONERROR);
				return;
			}

			editSession.chr = character;
			editSession.palName = pal->first;
			editSession.palette = &pal->second;
			tempSchema.destroy();
			ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_SCHEMA, ext == ".xml" ? ShadyCore::FileType::SCHEMA_XML : ShadyCore::FileType::SCHEMA_GAME_PATTERN})(&tempSchema, stream);
			try {
				object.reset();
				object = std::make_unique<EditableObject>(editSession.chr, tempSchema, *editSession.palette, editSession.palName);
				overlayData.object = std::make_unique<OverlayObject>(object->_moves);
				overlayData.object->displayScaled = true;
				loadedPath = path;
				loadLabels(character);
				loadLabel((std::filesystem::path(path).parent_path() / (character + "_labels.json")).string());
				refreshRightPanel(gui, object);
				bar->setMenuEnabled({"New"}, true);
				bar->setMenuEnabled({"Remove"}, true);
				bar->setMenuEnabled({"Misc"}, true);
				bar->setMenuItemEnabled({"File", "Export..."}, true);
				bar->setMenuItemEnabled({"File", "Import...", "palette..."}, true);
			} catch (std::exception &e) {
				Utils::dispMsg("Error", e.what(), MB_ICONERROR);
				loadedPath = path;
				refreshRightPanel(gui, object);
				bar->setMenuEnabled({"New"}, false);
				bar->setMenuEnabled({"Remove"}, false);
				bar->setMenuEnabled({"Misc"}, false);
				bar->setMenuItemEnabled({"File", "Export..."}, false);
				bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);
			}
			displayPalEditor(object, gui);
			win->close();
		});
		cancel->connect("Clicked", [win]{
			win->close();
		});
	};

	if (!object) {
		fct();
		return;
	}

	auto window = Utils::openWindowWithFocus(gui, 260, 90);

	window->loadWidgetsFromFile("assets/gui/quitConfirm.gui");
	window->get<tgui::Button>("Yes")->connect("Clicked", [&object, fct](std::weak_ptr<tgui::ChildWindow> self){
		self.lock()->close();
		if (saveFramedataToFileCallback(object))
			fct();
	}, std::weak_ptr<tgui::ChildWindow>(window));
	window->get<tgui::Button>("No")->connect("Clicked", [fct](std::weak_ptr<tgui::ChildWindow> self){
		self.lock()->close();
		fct();
	}, std::weak_ptr<tgui::ChildWindow>(window));
	window->get<tgui::Button>("Cancel")->connect("Clicked", [](std::weak_ptr<tgui::ChildWindow> self){
		self.lock()->close();
	}, std::weak_ptr<tgui::ChildWindow>(window));
}

void openPaletteFromFile(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr, tgui::Gui &gui)
{
	//TODO: Save last folder
	auto path = Utils::openFileDialog("Open palette file", ".", {{".*\\.pal", "Converted palette (.pal)"}, {".*\\.act", "Color table (.act)"}});

	if (path.empty())
		return;
	game->logger.info("Loading palette from file: " + path);

	auto entry = game->characterPaths[editSession.chr].palettes.find(path);
	ShadyCore::Palette *pal;

	// TODO: Don't reload only if the file wasn't modified
	if (entry == game->characterPaths[editSession.chr].palettes.end()) {
		std::ifstream stream(path, std::fstream::binary);
		std::string ext = path.substr(path.find_last_of('.') + 1);

		if (stream.fail()) {
			Utils::dispMsg("Error", "Could not open " + path + ": " + strerror(errno), MB_ICONERROR);
			return;
		}
		pal = &game->characterPaths[editSession.chr].palettes[path];
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_PALETTE, ext == "act" ? ShadyCore::FileType::PALETTE_ACT : ShadyCore::FileType::PALETTE_PAL})(pal, stream);
	} else
		pal = &entry->second;

	for (auto &action : object->_moves)
		for (auto &block : action.second)
			for (auto &anim : block)
				anim.setPalette(*pal, path);
	editSession.palette = pal;
	editSession.palName = path;
	displayPalEditor(object, gui);
}

void openPaletteFromPackage(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr bar, tgui::Gui &gui)
{
	auto window = Utils::openWindowWithFocus(gui, 300, 300);
	std::vector<std::string> pals;
	tgui::ListBox::Ptr list = tgui::ListBox::create();

	for (auto &pal : game->characterPaths[editSession.chr].palettes)
		pals.push_back(pal.first);

	window->setTitle("Select character's palette to load");
	std::sort(pals.begin(), pals.end());
	list->connect("DoubleClicked", [&object, bar, &gui, window](std::string palette){
		game->logger.info("Loading palette from package: " + palette);

		auto &pal = game->characterPaths[editSession.chr].palettes[palette];

		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(pal, palette);
		editSession.palette = &pal;
		editSession.palName = palette;
		window->close();
		displayPalEditor(object, gui);
	});
	for (auto &pal : pals)
		list->addItem(pal);
	list->setSize("&.w", "&.h");
	list->setScrollbarValue(0);
	window->add(list);
}

void openPaletteFromPalEditor(std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr, tgui::Gui &gui)
{
	//TODO: Save last folder
	auto path = Utils::openFileDialog("Open palette file", ".", {{".*\\.pal", "Palette editor palette"}});

	if (path.empty())
		return;
	game->logger.info("Loading palette from old pal editor file: " + path);

	struct stat s;

	if (stat(path.c_str(), &s) == -1) {
		Utils::dispMsg("Error", "Could not stat " + path + ": " + strerror(errno), MB_ICONERROR);
		return;
	}
	if (s.st_size < 1028) {
		Utils::dispMsg("Error", "File " + path + " is empty", MB_ICONERROR);
		return;
	}

	auto entry = game->characterPaths[editSession.chr].palettes.find(path);
	ShadyCore::Palette *pal;

	if (entry == game->characterPaths[editSession.chr].palettes.end()) {
		std::ifstream stream{path, std::ifstream::binary};

		if (stream.fail()) {
			Utils::dispMsg("Error", "Could not open file", MB_ICONERROR);
			return;
		}

		auto data = new uint32_t[256];
		auto metadata = new char[s.st_size - 1024];
		struct {
			const char *character;
			const char *name;
			const char *description;
			const char *author;
		} meta;

		stream.read((char *)data, 1024);
		stream.read(metadata, s.st_size - 1024);
		game->characterPaths[editSession.chr].palettes.erase(path);

		meta.character = metadata;
		game->logger.debug("Palette is meant for character " + std::string(meta.character));

		meta.name = meta.character + strlen(meta.character) + 1;
		game->logger.debug("  - Name " + std::string(meta.name));

		meta.description = meta.name + strlen(meta.name) + 1;
		game->logger.debug("  - Description " + std::string(meta.description));

		meta.author = meta.description + strlen(meta.description) + 1;
		game->logger.debug("  - Author " + std::string(meta.author));

		if (*meta.name || *meta.author || *meta.description)
			Utils::dispMsg("Metadata", "Palette " + std::string(meta.name) + " made by " + std::string(meta.author) + "\n\"" + meta.description + "\"\nPlease, be sure to credit the other when redistributing.", MB_ICONINFORMATION);
		pal = &game->characterPaths[editSession.chr].palettes[path];
		pal->initialize(16);

		auto palData = (uint16_t *)pal->data;

		memset(pal->data, 0, 512);
		for (int i = 0; i < 256; i++) {
			palData[i] |= (data[i] >> 19 & 0x1F) << 10;
			palData[i] |= (data[i] >> 11 & 0x1F) << 5;
			palData[i] |= (data[i] >> 3  & 0x1F) << 0;
			if (i)
				palData[i] |= 0x8000;
		}
		delete[] data;
		delete[] metadata;
	} else
		pal = &game->characterPaths[editSession.chr].palettes[path];

	for (auto &action : object->_moves)
		for (auto &block : action.second)
			for (auto &anim : block)
				anim.setPalette(*pal, path);
	editSession.palette = pal;
	editSession.palName = path;
	displayPalEditor(object, gui);
}

void saveFramedataToPackageCallback(std::unique_ptr<EditableObject> &)
{
	//TODO: Save
	Utils::dispMsg("Error", "Not implemented yet", MB_ICONERROR);
}

void savePaletteToFileCallback(std::unique_ptr<EditableObject> &)
{
	auto path = Utils::saveFileDialog(
		"Save palette file",
		editSession.palName.substr(0, 5) == "data/" ? "." :
		editSession.palName,
		{{".*\\.pal", "Converted palette (.pal)"}, {".*\\.act", "Color table (.act)"}}
	);

	if (path.empty())
		return;

	size_t pos = path.find_last_of('.');
	std::string ext = path.substr(pos);
	std::ofstream stream{path, std::fstream::binary};

	if (stream.fail()) {
		Utils::dispMsg("Error", "Could not open file " + path, MB_ICONERROR);
		return;
	}
	ShadyCore::getResourceWriter({ShadyCore::FileType::TYPE_PALETTE, ext == "act" ? ShadyCore::FileType::PALETTE_ACT : ShadyCore::FileType::PALETTE_PAL})(editSession.palette, stream);
}

void savePaletteToPackageCallback(std::unique_ptr<EditableObject> &)
{
	//TODO: Save
	Utils::dispMsg("Error", "Not implemented yet", MB_ICONERROR);
}

void	newFrameCallback(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr panel)
{
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
	auto it = object->_moves.at(object->_action)[object->_actionBlock].begin() + object->_animation;
	auto f = new ShadyCore::Schema::Sequence::MoveFrame(*data.frame);

	data.parent->frames.insert(data.parent->frames.begin() + object->_animation, f);
	object->_moves.at(object->_action)[object->_actionBlock].insert(it, FrameData(data, *f));

	auto anim = object->_animation + 1;
	auto oldBlock = object->_actionBlock;

	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");
	auto frame = panel->get<tgui::Slider>("Progress");

	block->setValue(oldBlock);
	frame->setValue(anim);
}

void	newEndFrameCallback(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr panel)
{
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
	auto it = object->_moves.at(object->_action)[object->_actionBlock].begin() + object->_animation;
	auto f = new ShadyCore::Schema::Sequence::MoveFrame(*data.frame);

	data.parent->frames.push_back(f);
	object->_moves.at(object->_action)[object->_actionBlock].push_back(FrameData(data, *f));

	auto oldBlock = object->_actionBlock;

	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");
	auto frame = panel->get<tgui::Slider>("Progress");

	block->setValue(oldBlock);
	frame->setValue(object->_moves.at(object->_action)[object->_actionBlock].size() - 1);
}

void	newAnimBlockCallback(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr panel)
{
	auto &action = object->_moves.at(object->_action);
	auto &data = action[object->_actionBlock][object->_animation];
	auto newSequence = new ShadyCore::Schema::Sequence(object->_action, false);
	auto newElem = new ShadyCore::Schema::Sequence::MoveFrame(*data.frame);

	newSequence->moveLock = data.parent->moveLock;
	newSequence->actionLock = data.parent->actionLock;
	newSequence->loop = data.parent->loop;
	newSequence->frames.push_back(newElem);

	size_t i = 0;
	auto it = tempSchema.objects.begin();

	while (i <= object->_actionBlock) {
		i += (*it)->getType() == 9 && reinterpret_cast<ShadyCore::Schema::Sequence *>(*it)->getId() == object->_action;
		it++;
	}
	tempSchema.objects.insert(it, newSequence);
	object->_actionBlock++;
	action.insert(action.begin() + object->_actionBlock, std::vector<FrameData>{});

	action[object->_actionBlock].emplace_back(editSession.chr, tempSchema, *newSequence, *newElem, *editSession.palette, editSession.palName);
	object->_animation = 0;
	refreshRightPanel(gui, object, false);

	auto block = panel->get<tgui::SpinButton>("Block");

	block->setMaximum(action.size() - 1);
	block->setValue(object->_actionBlock);
}

static bool msgDisplayed = false;
static bool msgDisplayed2 = false;

void	newHurtBoxCallback(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	auto &anim = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	if (anim.frame->hBoxes.size() >= 5 && !msgDisplayed) {
		Utils::dispMsg("Warning", "This frame already has 5 or more hurtboxes. The game can only process 5 hurtboxes at once and adding more than that may crash the game.", MB_ICONWARNING);
		msgDisplayed = true;
	}
	anim.frame->hBoxes.push_back({-10, -10, 10, 10});

	auto &box = anim.frame->hBoxes.back();

	refreshBoxes(boxes, anim, object);
	normalColor = {0x00, 0xFF, 0x00, 0xA0};
	selectBox(boxes->get<tgui::Button>("HurtBox" + std::to_string(anim.frame->hBoxes.size() - 1)), &box);
}

void	newHitBoxCallback(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	auto &anim = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	if (anim.frame->aBoxes.size() >= 5 && !msgDisplayed2) {
		Utils::dispMsg("Warning", "This frame already has 5 or more hitboxes. The game can only process 5 hitboxes at once and adding more than that may crash the game.", MB_ICONWARNING);
		msgDisplayed2 = true;
	}
	anim.frame->aBoxes.push_back({-10, -10, 10, 10});

	auto &box = anim.frame->aBoxes.back();

	refreshBoxes(boxes, anim, object);
	normalColor = {0xFF, 0x00, 0x00, 0xA0};
	selectBox(boxes->get<tgui::Button>("HitBox" + std::to_string(anim.frame->aBoxes.size() - 1)), &box);
}

void	removeFrameCallback(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	auto &arr = object->_moves.at(object->_action)[object->_actionBlock];

	if (arr.size() == 1) {
		*arr.back().frame = ShadyCore::Schema::Sequence::MoveFrame();
		refreshBoxes(boxes, arr.back(), object);
		selectBox(nullptr, nullptr);
		return;
	}
	arr[object->_animation].parent->frames.erase(arr[object->_animation].parent->frames.begin() + object->_animation);
	arr.erase(arr.begin() + object->_animation);

	if (object->_animation == arr.size())
		object->_animation--;
	refreshBoxes(boxes, arr[object->_animation], object);
	selectBox(nullptr, nullptr);
}

void	removeAnimationBlockCallback(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr panel)
{
	auto &arr = object->_moves.at(object->_action);

	if (arr.size() == 1)
		return;

	size_t i = 0;
	auto it = tempSchema.objects.begin();

	while (i < object->_actionBlock) {
		i += (*it)->getType() == 9 && reinterpret_cast<ShadyCore::Schema::Sequence *>(*it)->getId() == object->_action;
		it++;
	}
	tempSchema.objects.erase(it);
	arr.erase(arr.begin() + object->_actionBlock);
	if (object->_actionBlock == arr.size())
		object->_actionBlock--;

	auto block = panel->get<tgui::SpinButton>("Block");

	block->setMaximum(arr.size() - 1);
	block->setValue(object->_actionBlock);
}

void	removeActionCallback(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	if (object->_action == 0)
		return;

	auto it = object->_moves.find(object->_action);

	tempSchema.objects.erase(std::remove_if(tempSchema.objects.begin(), tempSchema.objects.end(), [&object](const ShadyCore::Schema::Object *o){
		if (o->getType() != 9)
			return false;

		auto s = reinterpret_cast<const ShadyCore::Schema::Sequence *>(o);

		return s->getId() == object->_action;
	}), tempSchema.objects.end());
	object->_moves.erase(it);
	refreshRightPanel(gui, object);
}

void	copyBoxesFromFrame(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes, FrameData &other)
{
	auto &frame = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	frame.frame->hBoxes = other.frame->hBoxes;
	frame.frame->aBoxes = other.frame->aBoxes;
	refreshBoxes(boxes, frame, object);
}

void	copyBoxesFromLastFrame(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	if (!object->_animation)
		return;
	copyBoxesFromFrame(object, boxes, object->_moves.at(object->_action)[object->_actionBlock][object->_animation - 1]);
}

void	copyBoxesFromNextFrame(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	if (object->_animation >= object->_moves.at(object->_action)[object->_actionBlock].size() - 1)
		return;
	copyBoxesFromFrame(object, boxes, object->_moves.at(object->_action)[object->_actionBlock][object->_animation + 1]);
}

void	flattenCollisionBoxes(std::unique_ptr<EditableObject> &, std::vector<std::vector<FrameData>> &action, FrameData *base)
{
	if (!base)
		for (auto &block : action)
			for (auto &frame : block)
				if (!frame.frame->cBoxes.empty()) {
					base = &frame;
					goto allGood;
				}
	if (!base || base->frame->cBoxes.empty())
		return;

allGood:
	for (auto &block : action)
		for (auto &frame : block)
			if (!frame.frame->cBoxes.empty() && &frame != base) {
				frame.frame->cBoxes.clear();
				frame.frame->cBoxes.push_back(base->frame->cBoxes.front());
			}
}

void	flattenThisMoveCollisionBoxes(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	flattenCollisionBoxes(object, object->_moves.at(object->_action), &object->_moves.at(object->_action)[object->_actionBlock][object->_animation]);
	refreshBoxes(std::move(boxes), object->_moves.at(object->_action)[object->_actionBlock][object->_animation], object);
}

void	flattenThisMoveProperties(std::unique_ptr<EditableObject> &object)
{
	auto &anim = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

	for (auto &a : object->_moves.at(object->_action)[object->_actionBlock])
		a.frame->traits = anim.frame->traits;
}

void	switchHitboxes(std::unique_ptr<EditableObject> &object, tgui::Panel::Ptr boxes)
{
	displayHitboxes = !displayHitboxes;
	if (object) {
		object->scale = 1;
		object->translate = {0, 0};
		overlayData.object->scale = 1;
		overlayData.object->translate = object->translate + overlayData.offset;
	}
	boxes->setVisible(displayHitboxes);
}

inline void displayPalEditorForce(std::unique_ptr<EditableObject> &object, tgui::Gui &gui)
{
	displayPalEditor(object, gui, true);
}

void	displayPalEditor(std::unique_ptr<EditableObject> &object, tgui::Gui &gui, bool force)
{
	auto f = std::make_shared<bool>(false);
	tgui::ChildWindow::Ptr window = gui.get<tgui::ChildWindow>("PaletteEditor");

	if (!window && !force)
		return;
	force = false;
	if (!window) {
		window = tgui::ChildWindow::create();
		window->setTitle("Palette editor");
		window->setPosition(200, 30);
		Utils::setRenderer(window);
		window->setSize(490, 450);
		force = true;
	}
	window->loadWidgetsFromFile("assets/gui/palEditor.gui");

	char buffer[8];
	auto pal = window->get<tgui::ComboBox>("Palettes");
	auto col = window->get<tgui::ScrollablePanel>("Colors");
	auto light = window->get<tgui::Slider>("Lightness");
	auto satHuePic = window->get<tgui::Picture>("SatHue");
	auto cross = window->get<tgui::Picture>("Cross");
	auto preview = window->get<tgui::TextBox>("Preview");
	auto hue = window->get<tgui::EditBox>("Hue");
	auto saturation = window->get<tgui::EditBox>("Sat");
	auto lightness = window->get<tgui::EditBox>("Light");
	auto edit = window->get<tgui::EditBox>("Edit");
	auto updatePalButton = [f, col, pal, &object](sf::Color bufferColor){
		if (*f)
			return;

		auto button = col->get<tgui::Button>("Color" + std::to_string(selectedColor));

		if (!button)
			return;
		if (!editSession.palette)
			return;

		auto renderer = button->getRenderer();
		auto &newpal = ((uint16_t*)editSession.palette->data)[selectedColor];

		newpal = 0;
		newpal |= (bufferColor.r >> 3 & 0x1F) << 10;
		newpal |= (bufferColor.g >> 3 & 0x1F) << 5;
		newpal |= (bufferColor.b >> 3  & 0x1F) << 0;
		if (selectedColor)
			newpal |= 0x8000;
		renderer->setBackgroundColor(bufferColor);
		renderer->setBackgroundColorDown(bufferColor);
		renderer->setBackgroundColorDisabled(bufferColor);
		renderer->setBackgroundColorFocused(bufferColor);
		renderer->setBackgroundColorHover(bufferColor);
		game->textureMgr.invalidatePalette(editSession.palName);
		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(*editSession.palette, editSession.palName);
	};
	auto updateTexture = [updatePalButton, light](Utils::HSLColor color, sf::Color bufferColor, bool modified) {
		sf::Image image;
		sf::Texture texture;

		image.create(light->getSize().x, light->getSize().y);

		auto size = image.getSize();

		for (unsigned y = 0; y < size.y; y++) {
			color.l = 240 - (240 * y / size.y);

			auto rgb = HSLtoRGB(color);
			sf::Color sf{rgb.r, rgb.g, rgb.b, 255};

			for (unsigned x = 0; x < size.x; x++)
				image.setPixel(x, y, sf);
		}
		texture.loadFromImage(image);
		light->getRenderer()->setTextureTrack({texture});
		if (modified)
			updatePalButton(bufferColor);
	};
	auto buttonCallBack = [edit, satHuePic, preview, light, hue, saturation, lightness, cross](tgui::Button::Ptr button){
		char buffer[8];
		auto c = button->getRenderer()->getBackgroundColor();
		sf::Color temp = {
			c.getRed(),
			c.getGreen(),
			c.getBlue()
		};
		Utils::HSLColor color = Utils::RGBtoHSL(temp);
		auto pos = satHuePic->getPosition();
		auto size = cross->getSize();

		sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
		hue->setText(std::to_string(color.h));
		saturation->setText(std::to_string(color.s));
		lightness->setText(std::to_string(color.l));
		cross->setPosition({
			(color.h * 200 / 240) + pos.x - size.x / 2,
			((240 - color.s) * 200 / 240) + pos.y - size.y / 2
		});
		edit->setText(buffer);
		light->setValue(color.l);
		preview->getRenderer()->setBackgroundColor(c);
	};
	auto sliderCallback = [updatePalButton, cross, light, lightness, satHuePic, hue, saturation, preview, edit]{
		char buffer[8];
		unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200;
		unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
		Utils::HSLColor color = {
			h,
			s,
			static_cast<unsigned char>(light->getValue())
		};
		sf::Color bufferColor = HSLtoRGB(color);

		sprintf(buffer, "#%02X%02X%02X", bufferColor.r, bufferColor.g, bufferColor.b);
		preview->getRenderer()->setBackgroundColor({bufferColor.r, bufferColor.g, bufferColor.b, 255});
		edit->setText(buffer);
		light->setValue(color.l);
		lightness->setText(std::to_string(color.l));
		updatePalButton(bufferColor);
	};

	for (auto &id : window->getWidgetNames())
		if (id.substring(0, strlen("Button")) == "Button") {
			auto button = window->get<tgui::Button>(id);

			button->connect("Clicked", buttonCallBack, button);
		}
	light->connect("ValueChanged", sliderCallback);
	edit->onReturnKeyPress.connect([f, cross, hue, saturation, light, lightness, satHuePic, edit, updateTexture]{
		std::string text = edit->getText();

		if (text.size() > 7) {
			edit->setText(text.substr(0, 7));
			return;
		} else if (text.size() != 7)
			return;

		tgui::Color color{edit->getText()};
		sf::Color bufferColor{
			color.getRed(),
			color.getGreen(),
			color.getBlue()
		};
		auto tmp = Utils::RGBtoHSL(bufferColor);
		auto pos = satHuePic->getPosition();
		auto size = cross->getSize();

		light->setValue(tmp.l);
		hue->setText(std::to_string(tmp.h));
		saturation->setText(std::to_string(tmp.s));
		lightness->setText(std::to_string(tmp.l));
		cross->setPosition({
			(tmp.h * 200 / 240.f) + pos.x - size.x / 2,
			((240 - tmp.s) * 200 / 240.f) + pos.y - size.y / 2
		});
		updateTexture(tmp, bufferColor, true);
	});
	hue->onReturnKeyPress.connect([cross, satHuePic, light, hue, saturation, lightness, edit, preview, updateTexture]{
		unsigned char h;

		try {
			auto nbr = std::stoul(hue->getText().toAnsiString());

			if (nbr > 240) {
				hue->setText("240");
				return;
			}
			h = nbr;
		} catch (...) { hue->setText("0"); return; }

		char buffer[8];
		unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
		Utils::HSLColor color = {
			h,
			s,
			static_cast<unsigned char>(light->getValue())
		};
		sf::Color temp = HSLtoRGB(color);
		auto pos = satHuePic->getPosition();
		auto size = cross->getSize();

		sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
		cross->setPosition({
			(h * 200 / 240) + pos.x - size.x / 2,
			cross->getPosition().y
		});
		edit->setText(buffer);
		preview->getRenderer()->setBackgroundColor({buffer});
		updateTexture(color, temp, true);
	});
	saturation->onReturnKeyPress.connect([cross, satHuePic, light, hue, saturation, lightness, edit, preview, updateTexture]{
		unsigned char s;

		try {
			auto nbr = std::stoul(saturation->getText().toAnsiString());

			if (nbr > 240) {
				saturation->setText("240");
				return;
			}
			s = nbr;
		} catch (...) { saturation->setText("0"); return; }

		char buffer[8];
		unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200;
		Utils::HSLColor color = {
			h,
			s,
			static_cast<unsigned char>(light->getValue())
		};
		sf::Color temp = HSLtoRGB(color);
		auto pos = satHuePic->getPosition();
		auto size = cross->getSize();

		sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
		cross->setPosition({
			(color.h * 200 / 240) + pos.x - size.x / 2,
			((240 - color.s) * 200 / 240) + pos.y - size.y / 2
		});
		edit->setText(buffer);
		preview->getRenderer()->setBackgroundColor({buffer});
		updateTexture(color, temp, true);
	});
	lightness->onReturnKeyPress.connect([updatePalButton, cross, satHuePic, light, hue, saturation, lightness, edit, preview]{
		unsigned char l;

		try {
			auto nbr = std::stoul(lightness->getText().toAnsiString());

			if (nbr > 240) {
				lightness->setText("0");
				return;
			}
			l = nbr;
		} catch (...) { lightness->setText("0"); return; }

		char buffer[8];
		unsigned char h = (cross->getSize().x / 2 + cross->getPosition().x - satHuePic->getPosition().x) * 240 / 200.;
		unsigned char s = 240 - (cross->getSize().y / 2 + cross->getPosition().y - satHuePic->getPosition().y) * 240 / 200;
		Utils::HSLColor color = {
			h,
			s,
			l
		};
		sf::Color temp = HSLtoRGB(color);

		sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
		light->setValue(l);
		edit->setText(buffer);

		tgui::Color tcolor{buffer};

		preview->getRenderer()->setBackgroundColor(tcolor);
		updatePalButton({
			tcolor.getRed(),
			tcolor.getGreen(),
			tcolor.getBlue(),
			tcolor.getAlpha()
		});
	});
	auto v = editSession.palette ? ((uint16_t *)editSession.palette->data)[selectedColor] : 0;
	sf::Color startColor{
		static_cast<sf::Uint8>((v & 0b0111110000000000) >> 7 | (v & 0b0111000000000000) >> 12),
		static_cast<sf::Uint8>((v & 0b0000001111100000) >> 2 | (v & 0b0000001110000000) >> 7),
		static_cast<sf::Uint8>((v & 0b0000000000011111) << 3 | (v & 0b0000000000011100) >> 2),
		static_cast<sf::Uint8>((v & 0x8000) ? 255 : 0)
	};
	Utils::HSLColor hslColor = Utils::RGBtoHSL(startColor);
	auto pos = satHuePic->getPosition();
	auto size = cross->getSize();

	sprintf(buffer, "#%02X%02X%02X", startColor.r, startColor.g, startColor.b);
	light->setValue(hslColor.l);
	hue->setText(std::to_string(hslColor.h));
	saturation->setText(std::to_string(hslColor.s));
	lightness->setText(std::to_string(hslColor.l));
	cross->setPosition({
		(hslColor.h * 200 / 240) + pos.x - size.x / 2,
		((240 - hslColor.s) * 200 / 240) + pos.y - size.y / 2
	});
	edit->setText(buffer);
	preview->getRenderer()->setBackgroundColor({buffer});
	updateTexture(hslColor, startColor, false);

	cross->ignoreMouseEvents();

	auto colorPickHandler = [cross, satHuePic, light, hue, saturation, edit, preview, updateTexture](tgui::Vector2f pos, bool b){
		char buffer[8];
		unsigned char h = pos.x * 240 / 200;
		unsigned char s = 240 - pos.y * 240 / 200;
		Utils::HSLColor color = {
			h,
			s,
			static_cast<unsigned char>(light->getValue())
		};
		sf::Color temp = HSLtoRGB(color);
		auto size = cross->getSize();

		sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
		hue->setText(std::to_string(color.h));
		saturation->setText(std::to_string(color.s));
		cross->setPosition({
			(color.h * 200 / 240) + satHuePic->getPosition().x - size.x / 2,
			((240 - color.s) * 200 / 240) + satHuePic->getPosition().y - size.y / 2
		});
		edit->setText(buffer);
		preview->getRenderer()->setBackgroundColor({buffer});
		updateTexture(color, temp, true);
		colorPickDrag = b;
	};

	satHuePic->connect("MousePressed",  [colorPickHandler](tgui::Vector2f pos){colorPickHandler(pos, true); });
	satHuePic->connect("MouseReleased", [colorPickHandler](tgui::Vector2f pos){colorPickHandler(pos, false);});

	col->removeAllWidgets();
	if (!editSession.palette) {
		gui.add(window, "PaletteEditor");
		pal->setEnabled(false);
		if (force)
			gui.add(window, "PaletteEditor");
		return;
	}

	auto lab = tgui::Label::create();

	lab->setPosition(0, 16 * 23);
	col->add(lab);
	for (auto &[name, _] : game->characterPaths[editSession.chr].palettes)
		pal->addItem(name);
	pal->setSelectedItem(editSession.palName);
	pal->connect("ItemSelected", [&object, &gui](const std::string &palette) {
		game->logger.info("Loading palette from package: " + palette);

		auto &pal = game->characterPaths[editSession.chr].palettes[palette];

		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(pal, palette);
		editSession.palette = &pal;
		editSession.palName = palette;
		displayPalEditor(object, gui);
	});
	for (int i = 0; i < 256; i++) {
		auto button = tgui::Button::create();
		auto renderer = button->getRenderer();
		auto val = ((uint16_t*)editSession.palette->data)[i];
		sf::Color color{
			static_cast<sf::Uint8>((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12),
			static_cast<sf::Uint8>((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7),
			static_cast<sf::Uint8>((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2),
			static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
		};

		button->setSize(20, 20);
		button->setPosition(5 + (i % 16) * 29.5, 5 + (i / 16) * 23);
		//TODO: Fix memory leaks like this (col is captured by copy so they are never actually freed)
		button->connect("Clicked", [col, satHuePic, i, pal, light, hue, saturation, lightness, cross, edit, preview, updateTexture](std::weak_ptr<tgui::Button> ptr){
			col->get<tgui::Button>("Color" + std::to_string(selectedColor))->setFocused(false);
			selectedColor = i;

			char buffer[8];
			auto val = ((uint16_t*)editSession.palette->data)[i];
			sf::Color color{
				static_cast<sf::Uint8>((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12),
				static_cast<sf::Uint8>((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7),
				static_cast<sf::Uint8>((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2),
				static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
			};
			Utils::HSLColor c = Utils::RGBtoHSL(color);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
			light->onValueChange.setEnabled(false);
			light->setValue(c.l);
			light->onValueChange.setEnabled(true);
			hue->setText(std::to_string(c.h));
			saturation->setText(std::to_string(c.s));
			lightness->setText(std::to_string(c.l));
			cross->setPosition({
				(c.h * 200 / 240) + pos.x - size.x / 2,
				((240 - c.s) * 200 / 240) + pos.y - size.y / 2
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
			updateTexture(c, color, false);
			ptr.lock()->setFocused(true);
		}, std::weak_ptr(button));
		button->connect("RightClicked", [col, satHuePic, i, pal, light, hue, saturation, lightness, cross, edit, preview, updateTexture](std::weak_ptr<tgui::Button> ptr){
			if (i == selectedColor)
				return;

			char buffer[8];
			auto val = ((uint16_t*)editSession.palette->data)[i];
			sf::Color color{
				static_cast<sf::Uint8>((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12),
				static_cast<sf::Uint8>((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7),
				static_cast<sf::Uint8>((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2),
				static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
			};
			Utils::HSLColor c = Utils::RGBtoHSL(color);
			auto pos = satHuePic->getPosition();
			auto size = cross->getSize();

			sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
			light->setValue(c.l);
			hue->setText(std::to_string(c.h));
			saturation->setText(std::to_string(c.s));
			lightness->setText(std::to_string(c.l));
			cross->setPosition({
				(c.h * 200 / 240) + pos.x - size.x / 2,
				((240 - c.s) * 200 / 240) + pos.y - size.y / 2
			});
			edit->setText(buffer);
			preview->getRenderer()->setBackgroundColor({buffer});
			updateTexture(c, color, true);
			ptr.lock()->setFocused(false);
			col->get<tgui::Button>("Color" + std::to_string(selectedColor))->setFocused(true);
		}, std::weak_ptr(button));
		button->connect("Unfocused", [i](std::weak_ptr<tgui::Button> self){
			if (i == selectedColor)
				self.lock()->setFocused(true);
		}, std::weak_ptr(button));
		button->connect("MouseLeft", [i]{
			hovering = false;
		});
		button->connect("MouseEntered", [i, &object]{
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
			const ShadyCore::Image *_img;
			auto val = ((uint16_t*)editSession.palette->data)[i];
			sf::Color color{
				static_cast<sf::Uint8>(~((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12)),
				static_cast<sf::Uint8>(~((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7)),
				static_cast<sf::Uint8>(~((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2)),
				static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
			};

			try {
				_img = &game->textureMgr.getUnderlyingImage("data/character/" + editSession.chr + "/" + data._schema->images.at(data.frame->imageIndex).name);
			} catch (std::out_of_range &e) {
				game->logger.debug("displayPalEditor:button[" + std::to_string(i) + "]: Invalid image: " + std::string(e.what()));
				return;
			}

			auto &img = *_img;

			hovering = true;
			if (img.bitsPerPixel != 8 )
				return;
			selectedColorMaskImage.create(img.width, img.height);
			for (unsigned y = 0; y < img.height; y++)
				for (unsigned x = 0; x < img.width; x++)
					if (i && img.raw[y * img.paddedWidth + x] == i)
						selectedColorMaskImage.setPixel(x, y, color);
					else
						selectedColorMaskImage.setPixel(x, y, sf::Color::Transparent);
			selectedColorMaskTexture.loadFromImage(selectedColorMaskImage);
			selectedColorMaskSprite.setTexture(selectedColorMaskTexture, true);
		});
		renderer->setBackgroundColor(color);
		renderer->setBackgroundColorDown(color);
		renderer->setBackgroundColorDisabled(color);
		renderer->setBackgroundColorFocused(color);
		renderer->setBackgroundColorHover(color);
		renderer->setBorderColorFocused(sf::Color::Red);
		button->setFocused(i == selectedColor);
		col->add(button, "Color" + std::to_string(i));
	}
	if (force)
		gui.add(window, "PaletteEditor");
}

void	displayBlendingOptions(tgui::Gui &)
{
	//TODO: make this a proper window
	Utils::dispMsg("Not implemented", "Blending options are not yet implemented", MB_ICONINFORMATION);
}

void	saveSettings()
{
	nlohmann::json json;

	json["swr"] = swr;
	json["soku"] = soku;
	json["soku2"] = soku2;
	json["extra"] = extra;
	json["bgColor"] = bgColor.toInteger();

	std::ofstream stream("assets/settings.json");

	stream << json;
}

void	displaySettings(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, tgui::MenuBar::Ptr bar)
{
	auto newSoku = std::make_shared<std::string>(soku);
	auto newSwr = std::make_shared<std::string>(swr);
	auto newSoku2 = std::make_shared<std::string>(soku2);
	auto newBgColor = std::make_shared<sf::Color>(bgColor);
	auto window = Utils::openWindowWithFocus(gui, 800, 370);

	window->loadWidgetsFromFile("assets/gui/settings.gui");
	window->setTitle("Settings");

	auto sokuPath = window->get<tgui::EditBox>("SokuPath");
	auto swrPath = window->get<tgui::EditBox>("SWRPath");
	auto soku2Path = window->get<tgui::EditBox>("Soku2Path");
	auto sokuBrowse = window->get<tgui::Button>("SokuBrowse");
	auto swrBrowse = window->get<tgui::Button>("SWRBrowse");
	auto soku2Browse = window->get<tgui::Button>("Soku2Browse");
	auto extraBrowse = window->get<tgui::Button>("ExtraBrowse");
	auto extraUp = window->get<tgui::BitmapButton>("ExtraUp");
	auto extraDown = window->get<tgui::BitmapButton>("ExtraDown");
	auto extraRemove = window->get<tgui::Button>("ExtraRemove");
	auto extraList = window->get<tgui::ListBox>("ExtraList");
	auto bg = window->get<tgui::Button>("BG");
	auto save = window->get<tgui::Button>("Save");
	auto cancel = window->get<tgui::Button>("Cancel");
	auto renderer = bg->getRenderer();
	auto setArrowColor = [](tgui::BitmapButton::Ptr button, sf::Color color){
		tgui::Texture texture = button->getImage();

		texture.setColor(color);
		button->setImage(texture);
	};

	for (auto &path : extra)
		extraList->addItem(path);
	sokuPath->setText(soku);
	swrPath->setText(swr);
	soku2Path->setText(soku2);

	renderer->setBackgroundColor(bgColor);
	renderer->setBackgroundColorDown(bgColor);
	renderer->setBackgroundColorDisabled(bgColor);
	renderer->setBackgroundColorFocused(bgColor);
	renderer->setBackgroundColorHover(bgColor);

	sokuBrowse->connect("Clicked", [newSoku, sokuPath]{
		auto path = Utils::openFileDialog("Select Soku dat", *newSoku + (char)std::filesystem::path::preferred_separator + "th123a.dat", {{"th123.\\.dat", "Touhou 12.3 dat file"}});

		if (path.empty())
			return;

		path = std::filesystem::path(path).parent_path().string();
		sokuPath->setText(path);
	});
	swrBrowse->connect("Clicked", [newSwr, swrPath]{
		auto path = Utils::openFileDialog("Select SWR dat", *newSwr + (char)std::filesystem::path::preferred_separator + "th105a.dat", {{"th105.\\.dat", "Touhou 10.5 dat file"}});

		if (path.empty())
			return;

		path = std::filesystem::path(path).parent_path().string();
		swrPath->setText(path);
	});
	soku2Browse->connect("Clicked", [newSoku2, soku2Path]{
		auto path = Utils::openFileDialog("Select Soku2 folder", newSoku2->substr(0, newSoku2->find_last_of(std::filesystem::path::preferred_separator)) + (char)std::filesystem::path::preferred_separator + "SOKU2.dll", {{"SOKU2\\.dll", "Soku2 mod file"}});

		if (path.empty())
			return;

		path = (std::filesystem::path(path).parent_path() / "characters").string();
		if (!std::filesystem::exists(path))
			Utils::dispMsg("Invalid Soku2 path", "Cannot access Soku2 characters folder. " + path + ": " + strerror(errno), MB_ICONEXCLAMATION);
		soku2Path->setText(path);
	});
	sokuPath->connect("TextChanged", [newSoku](const std::string &text){
		*newSoku = text;
	});
	swrPath->connect("TextChanged", [newSwr](const std::string &text){
		*newSwr = text;
	});
	soku2Path->connect("TextChanged", [newSoku2](const std::string &text){
		*newSoku2 = text;
	});
	bg->connect("Clicked", [&gui, newBgColor, renderer]{
		Utils::makeColorPickWindow(gui, [newBgColor, renderer](sf::Color color){
			*newBgColor = color;
			renderer->setBackgroundColor(color);
			renderer->setBackgroundColorDown(color);
			renderer->setBackgroundColorDisabled(color);
			renderer->setBackgroundColorFocused(color);
			renderer->setBackgroundColorHover(color);
		}, *newBgColor);
	});

	setArrowColor(extraUp, extraUp->getSharedRenderer()->getTextColorDisabled());
	setArrowColor(extraDown, extraDown->getSharedRenderer()->getTextColorDisabled());
	extraRemove->setEnabled(false);
	extraUp->setEnabled(false);
	extraDown->setEnabled(false);

	extraBrowse->connect("Clicked", [extraList]{
		auto path = Utils::openFileDialog("Select extra folder", ".", {{"^$", "Resource folder"}, {".*\\.dat", "Hisoutensoku package"}}, false, false, true);

		if (path.empty())
			return;
		path = std::filesystem::path(path).lexically_normal().string();
		extraList->addItem(path);
	});
	extraList->connect("ItemSelected", [setArrowColor, extraRemove, extraUp, extraDown, extraList]{
		const int index = extraList->getSelectedItemIndex();

		if (index >= 0)
			extraRemove->setEnabled(true);
		else
			extraRemove->setEnabled(false);

		if (index > 0) {
			setArrowColor(extraUp, extraUp->getSharedRenderer()->getTextColor());
			extraUp->setEnabled(true);
		} else {
			setArrowColor(extraUp, extraUp->getSharedRenderer()->getTextColorDisabled());
			extraUp->setEnabled(false);
		}

		if (index >= 0 && index + 1 < extraList->getItemCount()) {
			setArrowColor(extraDown, extraDown->getSharedRenderer()->getTextColor());
			extraDown->setEnabled(true);
		} else {
			setArrowColor(extraDown, extraDown->getSharedRenderer()->getTextColorDisabled());
			extraDown->setEnabled(false);
		}
	});
	extraUp->connect("Pressed", [extraList]{
		const std::size_t index = extraList->getSelectedItemIndex();
		sf::String value1 = extraList->getItemByIndex(index - 1);
		sf::String value2 = extraList->getItemByIndex(index);

		extraList->changeItemByIndex(index - 1, value2);
		extraList->changeItemByIndex(index, value1);
		extraList->setSelectedItemByIndex(index - 1);
	});
	extraDown->connect("Pressed", [extraList]{
		const std::size_t index = extraList->getSelectedItemIndex();
		sf::String value1 = extraList->getItemByIndex(index);
		sf::String value2 = extraList->getItemByIndex(index + 1);

		extraList->changeItemByIndex(index, value2);
		extraList->changeItemByIndex(index + 1, value1);
		extraList->setSelectedItemByIndex(index + 1);
	});
	extraRemove->connect("Pressed", [extraList]{
		const std::size_t index = extraList->getSelectedItemIndex();

		extraList->removeItemByIndex(index);
		if (extraList->getItemCount() > 0) {
			if (index == extraList->getItemCount())
				extraList->setSelectedItemByIndex(index - 1);
			else
				extraList->setSelectedItemByIndex(index);
		}
	});

	cancel->connect("Clicked", [](std::weak_ptr<tgui::ChildWindow> window){
		window.lock()->close();
	}, std::weak_ptr(window));
	save->connect("Clicked", [&gui, bar, &object, newSoku, newSwr, newSoku2, newBgColor, extraList](std::weak_ptr<tgui::ChildWindow> window){
		bgColor = *newBgColor;
		extra.clear();
		for (auto &ex : extraList->getItems())
			extra.push_back(ex);
		soku = *newSoku;
		swr = *newSwr;
		soku2 = *newSoku2;
		try {
			verifySettings();
		} catch (std::exception &e) {
			game->logger.error(e.what());
			return;
		}
		object.reset();
		loadedPath.clear();
		refreshRightPanel(gui, object);
		bar->setMenuEnabled({"New"}, false);
		bar->setMenuEnabled({"Remove"}, false);
		bar->setMenuEnabled({"Misc"}, false);
		bar->setMenuItemEnabled({"File", "Export..."}, false);
		bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);
		tempSchema.destroy();
		try {
			loadPackages();
		} catch (std::exception &e) {
			game->logger.error(e.what());
			Utils::dispMsg("Loading error", e.what(), MB_ICONERROR);
			return;
		}
		saveSettings();
		window.lock()->close();
	}, std::weak_ptr(window));
}

void	displayAbout(tgui::Gui &gui)
{
	auto window = Utils::openWindowWithFocus(gui, 470, 120);

	window->loadWidgetsFromFile("assets/gui/about.gui");
	window->setTitle("About FrameDataEditor");
	window->get<tgui::Label>("Version")->setText(VERSION_STR);
}

void	createPalette(tgui::Gui &gui)
{
	editSession.palName = "*" + editSession.palName;

	auto &pale = game->characterPaths[editSession.chr].palettes[editSession.palName];

	pale.initialize(editSession.palette->bitsPerPixel);
	memcpy(pale.data, editSession.palette->data, 512);
	editSession.palette = &game->characterPaths[editSession.chr].palettes[editSession.palName];

	tgui::ChildWindow::Ptr window = gui.get<tgui::ChildWindow>("PaletteEditor");

	if (!window)
		return;

	auto pal = window->get<tgui::ComboBox>("Palettes");

	pal->addItem(editSession.palName);
	pal->setSelectedItem(editSession.palName);
}

tgui::ChildWindow::Ptr	openExportWindow(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, const std::function<void (bool, bool, bool, bool, sf::Color)> &callback)
{
	auto win = Utils::openWindowWithFocus(gui, 300, 140);

	win->loadWidgetsFromFile("assets/gui/export_images.gui");

	auto gif = win->get<tgui::CheckBox>("Gif");
	auto cBox = win->get<tgui::CheckBox>("CBoxes");
	auto huBox = win->get<tgui::CheckBox>("HurtBoxes");
	auto hiBox = win->get<tgui::CheckBox>("HitBoxes");
	auto palette = win->get<tgui::ComboBox>("Palette");
	auto bgColorButton = win->get<tgui::Button>("Color");
	auto exportButton = win->get<tgui::Button>("Export");
	auto renderer = bgColorButton->getRenderer();
	auto weak = std::weak_ptr(win);

	for (auto &[name, _] : game->characterPaths[editSession.chr].palettes)
		palette->addItem(name);
	palette->setSelectedItem(editSession.palName);
	palette->connect("ItemSelected", [&object, &gui](const std::string &palette) {
		game->logger.info("Loading palette from loaded palette: " + palette);

		auto &pal = game->characterPaths[editSession.chr].palettes[palette];

		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(pal, palette);
		editSession.palette = &pal;
		editSession.palName = palette;
		displayPalEditor(object, gui);
	});
	exportButton->connect("Clicked", [weak, callback, renderer, hiBox, huBox, cBox, gif]{
		callback(gif->isChecked(), cBox->isChecked(), huBox->isChecked(), hiBox->isChecked(), renderer->getBackgroundColor());
		weak.lock()->close();
	});
	bgColorButton->connect("Clicked", [renderer, &gui]{
		Utils::makeColorPickWindow(gui, [renderer](sf::Color color){
			renderer->setBackgroundColor(color);
			renderer->setBackgroundColorDisabled(color);
			renderer->setBackgroundColorHover(color);
			renderer->setBackgroundColorDown(color);
			renderer->setBackgroundColorFocused(color);
		}, renderer->getBackgroundColor())->setTitle("Select background color");
	});
	bgColorButton->connect("RightClicked", [renderer]{
		auto color = sf::Color::Transparent;

		renderer->setBackgroundColor(color);
		renderer->setBackgroundColorDisabled(color);
		renderer->setBackgroundColorHover(color);
		renderer->setBackgroundColorDown(color);
		renderer->setBackgroundColorFocused(color);
	});
	// TODO:
	gif->setEnabled(false);
	return win;
}

sf::Image createExportImage(std::unique_ptr<EditableObject> &object, bool cb, bool hurtb, bool hitb, sf::Color bgColor)
{
	auto spriteData = object->setupSprite();
	auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
	auto botright = spriteData.first + spriteData.second;

	if (cb && !data.frame->cBoxes.empty()) {
		auto &box = data.frame->cBoxes[0];

		spriteData.first.x = std::min(spriteData.first.x, box.left);
		spriteData.first.y = std::min(spriteData.first.y, box.up);
		botright.x = std::max(botright.x, box.right);
		botright.y = std::max(botright.y, box.down);
	}
	if (hurtb)
		for (auto &box : data.frame->hBoxes) {
			spriteData.first.x = std::min(spriteData.first.x, box.left);
			spriteData.first.y = std::min(spriteData.first.y, box.up);
			botright.x = std::max(botright.x, box.right);
			botright.y = std::max(botright.y, box.down);
		}
	if (hitb)
		for (auto &box : data.frame->aBoxes) {
			spriteData.first.x = std::min(spriteData.first.x, box.left);
			spriteData.first.y = std::min(spriteData.first.y, box.up);
			botright.x = std::max(botright.x, box.right);
			botright.y = std::max(botright.y, box.down);
		}
	spriteData.second = botright - spriteData.first;

	sf::RenderStates state;
	sf::Image img;
	sf::RectangleShape rect;
	sf::RenderTexture texture;

	state.transform.translate(-spriteData.first.x + 10, -spriteData.first.y + 10);
	texture.create(spriteData.second.x + 20, spriteData.second.y + 20);
	texture.clear(bgColor);
	if (!data.frame->hasBlendOptions() || data.frame->blendOptions.mode == 0);
	else if (data.frame->blendOptions.mode == 1) {
		if (!bgColor.a) {
			auto a = object->_sprite.getTexture()->copyToImage();

			for (unsigned x = 0; x < a.getSize().x; x++)
				for (unsigned y = 0; y < a.getSize().y; y++) {
					auto color = a.getPixel(x, y);

					color.a = color.a * std::max(color.r, std::max(color.g, color.b)) / 255;
					a.setPixel(x, y, color);
				}
			tempTexture.loadFromImage(a);
			object->_sprite.setTexture(tempTexture);
		} else
			state.blendMode = sf::BlendAdd;
	} else if (data.frame->blendOptions.mode == 2) {
		if (!bgColor.a) {
			auto a = object->_sprite.getTexture()->copyToImage();

			for (unsigned x = 0; x < a.getSize().x; x++)
				for (unsigned y = 0; y < a.getSize().y; y++) {
					auto color = a.getPixel(x, y);

					color.a = color.a * std::max(color.r, std::max(color.g, color.b)) / 255;
					color.r = 255 - color.r;
					color.g = 255 - color.g;
					color.b = 255 - color.b;
					a.setPixel(x, y, color);
				}
			tempTexture.loadFromImage(a);
			object->_sprite.setTexture(tempTexture);
		} else
			state.blendMode = sf::BlendMode{
				sf::BlendMode::Zero,
				sf::BlendMode::OneMinusSrcColor,
				sf::BlendMode::Add,
				sf::BlendMode::One,
				sf::BlendMode::One,
				sf::BlendMode::Add
			};
	}
	texture.draw(object->_sprite, state);
	rect.setOutlineThickness(1);
	if (hurtb) {
		for (auto &box : data.frame->hBoxes) {
			rect.setPosition(-spriteData.first.x + box.left + 10, -spriteData.first.y + box.up + 10);
			if (box.right == box.left || box.down == box.up) {
				auto size = sf::Vector2f{
					static_cast<float>(box.right - box.left),
					static_cast<float>(box.down - box.up)
				};

				size.x = std::max(size.x, 1.f);
				size.y = std::max(size.y, 1.f);
				rect.setFillColor(sf::Color{0x00, 0xFF, 0x00});
				rect.setOutlineColor(sf::Color::Transparent);
				rect.setSize(size);
			} else {
				rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
				rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00});
				rect.setSize({
					static_cast<float>(box.right - box.left),
					static_cast<float>(box.down - box.up)
				});
			}
			texture.draw(rect);
		}
	}
	if (hitb) {
		for (auto &box : data.frame->aBoxes) {
			rect.setPosition(-spriteData.first.x + box.left + 10, -spriteData.first.y + box.up + 10);
			if (box.right == box.left || box.down == box.up) {
				auto size = sf::Vector2f{
					static_cast<float>(box.right - box.left),
					static_cast<float>(box.down - box.up)
				};

				size.x = std::max(size.x, 1.f);
				size.y = std::max(size.y, 1.f);
				rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00});
				rect.setOutlineColor(sf::Color::Transparent);
				rect.setSize(size);
			} else {
				rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
				rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00});
				rect.setSize({
					static_cast<float>(box.right - box.left),
					static_cast<float>(box.down - box.up)
				});
			}
			texture.draw(rect);
		}
	}
	if (cb && !data.frame->cBoxes.empty()) {
		auto &box = data.frame->cBoxes[0];

		rect.setPosition(-spriteData.first.x + box.left + 10, -spriteData.first.y + box.up + 10);
		rect.setSize({
			static_cast<float>(box.right - box.left),
			static_cast<float>(box.down - box.up)
		});
		rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
		rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00});
		texture.draw(rect);
	}
	texture.display();
	return texture.getTexture().copyToImage();
}

void	exportFrameToImage(std::unique_ptr<EditableObject> &object, bool cb, bool hurtb, bool hitb, sf::Color bgColor)
{
	auto imagePath = std::filesystem::path{"exported"} / editSession.chr / std::to_string(object->_action);
	auto name = std::to_string(object->_actionBlock) + "_" + std::to_string(object->_animation) + ".png";

	std::filesystem::create_directories(imagePath);
	createExportImage(object, cb, hurtb, hitb, bgColor).saveToFile((imagePath / name).string().c_str());
}

void	exportThisFrame(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	openExportWindow(gui, object, [&object](bool gif, bool cb, bool hurtb, bool hitb, sf::Color bgColor){
		exportFrameToImage(object, cb, hurtb, hitb, bgColor);
	})->get<tgui::CheckBox>("Gif")->setEnabled(false);
}

void	exportThisMove(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	openExportWindow(gui, object, [&gui, &object](bool gif, bool cb, bool hurtb, bool hitb, sf::Color bgColor){
		gui.get<tgui::Panel>("Panel1")->get<tgui::SpinButton>("Speed")->setValue(0);

		auto &act = object->_moves.at(object->_action);
		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 75});
		gui.add(panel);

		auto window = tgui::ChildWindow::create("Exporting...", tgui::ChildWindow::TitleButton::None);

		window->setSize(360, 90);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window, "LoadingWindow");

		window->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, window, panel, tabUsageEnabled]{
			gui.remove(window);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};
		unsigned x = 0;

		for (const auto &i : act)
			x += i.size();
		Utils::setRenderer(window);
		window->loadWidgetsFromFile("assets/gui/loading.gui");

		auto bar = window->get<tgui::ProgressBar>("ProgressBar");
		auto progress = window->get<tgui::Label>("Progress");
		auto label = window->get<tgui::Label>("Label");
		auto doIt = [&object, &act, cb, hurtb, hitb, bgColor, bar, progress, x, label] {
			auto block = object->_actionBlock;
			auto anim = object->_animation;
			unsigned current = 0;
			auto name = actionNames.find(static_cast<SokuLib::Action>(object->_action));
			auto action = parseActionName(name == actionNames.end() ? "Action #" + std::to_string(object->_action) : name->second, label);

			for (unsigned i = 0; i < act.size(); i++) {
				auto &blk = act[i];

				object->_actionBlock = i;
				for (unsigned j = 0; j < blk.size(); j++) {
					uiTasks.emplace_back([&object, label, action]{
						label->setText(action + "  Block " + std::to_string(object->_actionBlock) + "  Animation " + std::to_string(object->_animation));
					});

					object->_animation = j;
					exportFrameToImage(object, cb, hurtb, hitb, bgColor);
					current++;

					uiTasks.emplace_back([current, bar, progress, x]{
						bar->setValue(current);
						progress->setText(std::to_string(current) + "/" + std::to_string(x));
					});
				}
			}
			object->_actionBlock = block;
			object->_animation = anim;
		};

		bar->setMaximum(x);
		progress->setText("0/" + std::to_string(x));
		label->setText("Exporting data....");
		std::thread{[doIt, closeWindow]{
			doIt();
			closeWindow();
		}}.detach();
	});
}

void	exportAll(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	openExportWindow(gui, object, [&object, &gui](bool gif, bool cb, bool hurtb, bool hitb, sf::Color bgColor){
		gui.get<tgui::Panel>("Panel1")->get<tgui::SpinButton>("Speed")->setValue(0);

		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 75});
		gui.add(panel);

		auto window = tgui::ChildWindow::create("Exporting...", tgui::ChildWindow::TitleButton::None);

		window->setSize(360, 90);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window, "LoadingWindow");

		window->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, window, panel, tabUsageEnabled]{
			gui.remove(window);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};
		unsigned x = 0;

		for (auto &[id, act]: object->_moves)
			for (const auto &i : act)
				x += i.size();
		Utils::setRenderer(window);
		window->loadWidgetsFromFile("assets/gui/loading.gui");

		auto bar = window->get<tgui::ProgressBar>("ProgressBar");
		auto progress = window->get<tgui::Label>("Progress");
		auto label = window->get<tgui::Label>("Label");
		auto doIt = [&object, cb, hurtb, hitb, bgColor, bar, progress, x, label] {
			auto anim = object->_animation;
			auto block = object->_actionBlock;
			auto oldAction = object->_action;
			unsigned current = 0;

			for (auto &[id, act]: object->_moves) {
				object->_action = id;

				auto name = actionNames.find(static_cast<SokuLib::Action>(object->_action));
				auto action = parseActionName(name == actionNames.end() ? "Action #" + std::to_string(object->_action) : name->second, label);

				for (unsigned i = 0; i < act.size(); i++) {
					auto &blk = act[i];

					object->_actionBlock = i;
					for (unsigned j = 0; j < blk.size(); j++) {
						uiTasks.emplace_back([&object, label, action]{
							label->setText(action + "  Block " + std::to_string(object->_actionBlock) + "  Animation " + std::to_string(object->_animation));
						});

						object->_animation = j;
						exportFrameToImage(object, cb, hurtb, hitb, bgColor);
						current++;

						uiTasks.emplace_back([current, bar, progress, x]{
							bar->setValue(current);
							progress->setText(std::to_string(current) + "/" + std::to_string(x));
						});
					}
				}
			}
			object->_actionBlock = block;
			object->_animation = anim;
			object->_action = oldAction;
		};

		bar->setMaximum(x);
		progress->setText("0/" + std::to_string(x));
		label->setText("Exporting data....");
		std::thread{[doIt, closeWindow]{
			doIt();
			uiTasks.emplace_back([closeWindow] {
				closeWindow();
			});
		}}.detach();
	});
}

void overlayMove(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	auto window = gui.get<tgui::ChildWindow>("Overlay move");

	if (window) {
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		window->setFocused(true);
		return;
	}

	window = tgui::ChildWindow::create();
	window->setSize(320, 120);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
	window->setTitle("Overlay move");
	gui.add(window, "Overlay move");
	Utils::setRenderer(window);
	window->connect("Closed", [&gui, window]{
		gui.remove(window);
	});
	window->loadWidgetsFromFile("assets/gui/overlay.gui");

	auto display = window->get<tgui::CheckBox>("Display");
	auto front = window->get<tgui::CheckBox>("Front");
	auto play = window->get<tgui::CheckBox>("Play");
	auto offset = window->get<tgui::EditBox>("Offset");
	auto action = window->get<tgui::EditBox>("Action");
	auto block = window->get<tgui::SpinButton>("Block");
	auto frame = window->get<tgui::Slider>("Frame");
	auto blockLabel = window->get<tgui::Label>("BlockLabel");
	auto frameLabel = window->get<tgui::Label>("FrameLabel");

	display->setChecked(overlayData.display);
	front->setChecked(overlayData.front);
	play->setChecked(overlayData.play);
	offset->setText("(" + std::to_string(overlayData.offset.x) + "," + std::to_string(-overlayData.offset.y) + ")");
	action->setText(std::to_string(overlayData.object->_action));
	if (overlayData.object->_moves.count(overlayData.object->_action)) {
		block->setMaximum(overlayData.object->_moves.at(overlayData.object->_action).size() - 1);
		block->setValue(overlayData.object->_actionBlock);
		if (overlayData.object->_actionBlock < overlayData.object->_moves.at(overlayData.object->_action).size()) {
			frame->setMaximum(overlayData.object->_moves.at(overlayData.object->_action)[overlayData.object->_actionBlock].size() - 1);
			frame->setValue(overlayData.object->_animation);
		}
	}
	blockLabel->setText("Block " + std::to_string(overlayData.object->_actionBlock + 1) + "/" + std::to_string(overlayData.object->_moves[overlayData.object->_action].size()));
	frameLabel->setText("Frame " + std::to_string(overlayData.object->_animation));

	display->connect("Changed", [] (bool f){
		overlayData.display = f;
	});
	front->connect("Changed", [] (bool f){
		overlayData.front = f;
	});
	play->connect("Changed", [] (bool f){
		overlayData.play = f;
	});
	offset->connect("TextChanged", [&object](const std::string &t) {
		if (t.empty())
			return;

		auto pos = t.find(',');
		auto x = t.substr(1, pos - 1);

		auto remainder = t.substr(pos + 1);
		auto y = remainder.substr(0, remainder.size() - 1);

		try {
			std::stoul(x);
			overlayData.offset.x = std::stol(x);
			overlayData.offset.y = -std::stol(y);
			overlayData.object->translate = object->translate + overlayData.offset;
		} catch (...) {}
	});
	action->connect("TextChanged", [block, frame, blockLabel, frameLabel](const std::string &t) {
		try {
			overlayData.object->_action = std::stoul(t);
			overlayData.object->_actionBlock = 0;
			overlayData.object->_animation = 0;
			overlayData.object->_animationCtr = 0;
			*c = true;
			if (overlayData.object->_moves.count(overlayData.object->_action)) {
				block->setMaximum(overlayData.object->_moves.at(overlayData.object->_action).size() - 1);
				block->setValue(overlayData.object->_actionBlock);
				blockLabel->setText("Block " + std::to_string(overlayData.object->_actionBlock + 1) + "/" + std::to_string(overlayData.object->_moves[overlayData.object->_action].size()));
				if (overlayData.object->_actionBlock < overlayData.object->_moves.at(overlayData.object->_action).size()) {
					frame->setMaximum(overlayData.object->_moves.at(overlayData.object->_action)[overlayData.object->_actionBlock].size() - 1);
					frame->setValue(overlayData.object->_animation);
					frameLabel->setText("Frame " + std::to_string(overlayData.object->_animation));
				}
			}
			*c = false;
		} catch (...) {}
	});
	block->connect("ValueChanged", [frame, frameLabel, blockLabel](float f){
		if (*c)
			return;
		overlayData.object->_actionBlock = f;
		overlayData.object->_animation = 0;
		overlayData.object->_animationCtr = 0;
		*c = true;
		if (overlayData.object->_moves.count(overlayData.object->_action)) {
			blockLabel->setText("Block " + std::to_string(overlayData.object->_actionBlock + 1) + "/" + std::to_string(overlayData.object->_moves[overlayData.object->_action].size()));
			if (overlayData.object->_actionBlock < overlayData.object->_moves.at(overlayData.object->_action).size()) {
				frame->setMaximum(overlayData.object->_moves.at(overlayData.object->_action)[overlayData.object->_actionBlock].size() - 1);
				frame->setValue(overlayData.object->_animation);
				frameLabel->setText("Frame " + std::to_string(overlayData.object->_animation));
			}
		}
		*c = false;
	});
	frame->connect("ValueChanged", [frameLabel](float f){
		if (*c)
			return;
		overlayData.object->_animation = f;
		overlayData.object->_animationCtr = 0;
		frameLabel->setText("Frame " + std::to_string(overlayData.object->_animation));
	});
}

void	placeGuiHooks(tgui::Gui &gui, std::unique_ptr<EditableObject> &object)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto boxes = gui.get<tgui::Panel>("Boxes");

	game->logger.debug("Placing hooks");
	placeAnimPanelHooks(gui, panel, boxes, object);

	bar->setMenuEnabled({"New"}, false);
	bar->setMenuEnabled({"Remove"}, false);
	bar->setMenuEnabled({"Misc"}, false);
	bar->setMenuItemEnabled({"File", "Export..."}, false);
	bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);

	bar->connectMenuItem({"File", "Import...", "framedata...", "from file"}, openFramedataFromFile, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Import...", "framedata...", "from game package"}, openFramedataFromPackage, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Import...", "palette...", "from file"}, openPaletteFromFile, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Import...", "palette...", "from loaded palettes"}, openPaletteFromPackage, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Import...", "palette...", "from palette editor format"}, openPaletteFromPalEditor, std::ref(object), bar, std::ref(gui));
	bar->connectMenuItem({"File", "Export...", "framedata...", "to file"}, saveFramedataToFileCallback, std::ref(object));
	bar->connectMenuItem({"File", "Export...", "framedata...", "to game package"}, saveFramedataToPackageCallback, std::ref(object));
	bar->connectMenuItem({"File", "Export...", "palette...", "to file"}, savePaletteToFileCallback, std::ref(object));
	bar->connectMenuItem({"File", "Export...", "palette...", "to game package"}, savePaletteToPackageCallback, std::ref(object));
	bar->connectMenuItem({"File", "Settings"}, displaySettings, std::ref(gui), std::ref(object), bar);
	bar->connectMenuItem({"File", "Quit (Ctrl + Q)"}, quitCallback);
	bar->connectMenuItem({"File", "New framedata (Ctrl + N)"}, newFileCallback, std::ref(object), bar, std::ref(gui));

	bar->connectMenuItem({"New", "Palette"}, createPalette, std::ref(gui));
	bar->connectMenuItem({"New", "Frame (Ctrl + F)"}, newFrameCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "End frame (Ctrl + Shift + F)"}, newEndFrameCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "Animation block (Ctrl + B)"}, newAnimBlockCallback, std::ref(gui), std::ref(object), panel);
	bar->connectMenuItem({"New", "Hurt box (Ctrl + H)"}, newHurtBoxCallback, std::ref(object), boxes);
	bar->connectMenuItem({"New", "Hit box (Ctrl + Shift + H)"}, newHitBoxCallback, std::ref(object), boxes);

	bar->connectMenuItem({"Remove", "Frame (Ctrl + Shift + Del)"}, removeFrameCallback, std::ref(object), boxes);
	bar->connectMenuItem({"Remove", "Animation block (Shift + Del)"}, removeAnimationBlockCallback, std::ref(object), panel);
	bar->connectMenuItem({"Remove", "Action (Ctrl + Del)"}, removeActionCallback, std::ref(gui), std::ref(object));
	bar->connectMenuItem({"Remove", "Selected box (Del)"}, removeBoxCallback, boxes, std::ref(object), panel);

	bar->connectMenuItem({"Misc", "Export image", "Export this frame"}, exportThisFrame, std::ref(gui), std::ref(object));
	bar->connectMenuItem({"Misc", "Export image", "Export this move frames"}, exportThisMove, std::ref(gui), std::ref(object));
	bar->connectMenuItem({"Misc", "Export image", "Export all this character frames"}, exportAll, std::ref(gui), std::ref(object));

	bar->connectMenuItem({"Misc", "Copy boxes from last frame"}, copyBoxesFromLastFrame, std::ref(object), boxes);
	bar->connectMenuItem({"Misc", "Copy boxes from next frame"}, copyBoxesFromNextFrame, std::ref(object), boxes);
	bar->connectMenuItem({"Misc", "Flatten this move collision boxes"}, flattenThisMoveCollisionBoxes, std::ref(object), boxes);
	bar->connectMenuItem({"Misc", "Flatten this move properties"}, flattenThisMoveProperties, std::ref(object));
	bar->connectMenuItem({"Misc", "Reverse palette index"}, [&gui, &object]{
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		for (int i = 0; i < 128; i++)
			std::swap(((uint16_t*)editSession.palette->data)[i], ((uint16_t*)editSession.palette->data)[255 - i]);
		((uint16_t*)editSession.palette->data)[0] &= 0x7FFF;
		((uint16_t*)editSession.palette->data)[255] |= 0x8000;
		game->textureMgr.invalidatePalette(editSession.palName);
		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(*editSession.palette, editSession.palName);
		displayPalEditor(object, gui, false);
	});
	bar->connectMenuItem({"Misc", "Evil mode"}, [&gui, &object]{
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		for (int i = 0; i < 256; i++) {
			auto &val = ((uint16_t*)editSession.palette->data)[i];

			val = ~val;
			if (i)
				val |= 0x8000;
			else
				val &= ~0x8000;
		}
		game->textureMgr.invalidatePalette(editSession.palName);
		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block)
					anim.setPalette(*editSession.palette, editSession.palName);
		displayPalEditor(object, gui, false);
	});
	bar->connectMenuItem({"Misc", "Reload textures"}, [&object]{
		game->logger.debug("Reloading textures");

		for (auto &e : extraPackages) {
			game->package.demerge(e);
			delete e;
		}
		extraPackages.clear();
		game->textureMgr.invalidateAll();
		for (auto &path : extra)
			extraPackages.push_back(new ShadyCore::Package(path));
		for (auto &path : extraPackages)
			game->package.merge(path);

		for (auto &action : object->_moves)
			for (auto &block : action.second)
				for (auto &anim : block) {
					anim.textureHandle = 0;
					anim.reloadTexture();
				}
	});
	bar->connectMenuItem({"Misc", "Reload palettes"}, []{
		game->logger.debug("Reloading palettes");
		Utils::dispMsg("Not implemented", "Reloading palettes is not yet implemented", MB_ICONINFORMATION);
	});

	bar->connectMenuItem({"View", "Palette editor"}, displayPalEditorForce, std::ref(object), std::ref(gui));
	bar->connectMenuItem({"View", "Blending options"}, displayBlendingOptions, std::ref(gui));
	bar->connectMenuItem({"View", "Hitboxes (H)"}, switchHitboxes, std::ref(object), boxes);
	bar->connectMenuItem({"View", "Overlay move"}, overlayMove, std::ref(gui), std::ref(object));

	bar->connectMenuItem({"Help", "About"}, displayAbout, std::ref(gui));

	for (unsigned i = 0; i < resizeButtons.size(); i++) {
		auto &resizeButton = resizeButtons[i];

		resizeButton = tgui::Button::create();
		resizeButton->setVisible(false);
		resizeButton->setSize(8, 8);
		resizeButton->getRenderer()->setBackgroundColor(tgui::Color{0xA0, 0xA0, 0xA0});
		resizeButton->getRenderer()->setBorderColor(tgui::Color::Red);
		resizeButton->connect("MousePressed", [i]{
			canDrag = true;
			dragUp = i < 3;
			dragDown = i > 4;
			dragLeft = i == 0 || i == 3 || i == 5;
			dragRight = i == 2 || i == 4 || i == 7;
		});
		gui.add(resizeButton);
	}
}

void	handleColorPickDrag(tgui::Gui &gui, std::unique_ptr<EditableObject> &object, int mouseX, int mouseY)
{
	tgui::ChildWindow::Ptr window = gui.get<tgui::ChildWindow>("PaletteEditor");

	if (!window)
		return;

	auto satHuePic = window->get<tgui::Picture>("SatHue");
	auto picPos = satHuePic->getAbsolutePosition();
	auto pos = Vector2f{mouseX - picPos.x, mouseY - picPos.y};
	auto pal = window->get<tgui::ComboBox>("Palettes");
	auto col = window->get<tgui::ScrollablePanel>("Colors");
	auto light = window->get<tgui::Slider>("Lightness");
	auto cross = window->get<tgui::Picture>("Cross");
	auto preview = window->get<tgui::TextBox>("Preview");
	auto hue = window->get<tgui::EditBox>("Hue");
	auto saturation = window->get<tgui::EditBox>("Sat");
	auto lightness = window->get<tgui::EditBox>("Light");
	auto edit = window->get<tgui::EditBox>("Edit");
	char buffer[8];
	unsigned char h = std::max(0.f, std::min(240.f, pos.x * 240 / 200));
	unsigned char s = std::max(0.f, std::min(240.f, 240 - pos.y * 240 / 200));
	Utils::HSLColor color = {
		h,
		s,
		static_cast<unsigned char>(light->getValue())
	};
	sf::Color temp = HSLtoRGB(color);
	auto csize = cross->getSize();
	sf::Image image;
	sf::Texture texture;

	sprintf(buffer, "#%02X%02X%02X", temp.r, temp.g, temp.b);
	hue->setText(std::to_string(color.h));
	saturation->setText(std::to_string(color.s));
	cross->setPosition({
		(color.h * 200 / 240) + satHuePic->getPosition().x - csize.x / 2,
		((240 - color.s) * 200 / 240) + satHuePic->getPosition().y - csize.y / 2
	});
	edit->setText(buffer);
	preview->getRenderer()->setBackgroundColor({buffer});

	image.create(light->getSize().x, light->getSize().y);

	auto size = image.getSize();

	for (unsigned y = 0; y < size.y; y++) {
		color.l = 240 - (240 * y / size.y);

		auto rgb = HSLtoRGB(color);
		sf::Color sf{rgb.r, rgb.g, rgb.b, 255};

		for (unsigned x = 0; x < size.x; x++)
			image.setPixel(x, y, sf);
	}
	texture.loadFromImage(image);
	light->getRenderer()->setTextureTrack({texture});

	auto button = col->get<tgui::Button>("Color" + std::to_string(selectedColor));

	if (!button)
		return;
	if (!editSession.palette)
		return;

	auto renderer = button->getRenderer();
	auto &newpal = ((uint16_t*)editSession.palette->data)[selectedColor];

	newpal = 0;
	newpal |= (temp.r >> 3 & 0x1F) << 10;
	newpal |= (temp.g >> 3 & 0x1F) << 5;
	newpal |= (temp.b >> 3  & 0x1F) << 0;
	if (selectedColor)
		newpal |= 0x8000;
	renderer->setBackgroundColor(temp);
	renderer->setBackgroundColorDown(temp);
	renderer->setBackgroundColorDisabled(temp);
	renderer->setBackgroundColorFocused(temp);
	renderer->setBackgroundColorHover(temp);
	game->textureMgr.invalidatePalette(editSession.palName);
	for (auto &action : object->_moves)
		for (auto &block : action.second)
			for (auto &anim : block)
				anim.setPalette(*editSession.palette, editSession.palName);
}

void	handleDrag(tgui::Gui &, std::unique_ptr<EditableObject> &object, int mouseX, int mouseY)
{
	if (!object)
		return;
	if (!selectedBox && !spriteSelected)
		return;
	if (mouseStart.distance(Vector2i{mouseX, mouseY}) > 10)
		dragStart = true;
	if (!dragStart)
		return;
	if (!dragLeft && !dragRight && !dragUp && !dragDown) {
		if (spriteSelected) {/*
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];
			Vector2i diff = Vector2i{mouseX, mouseY} - lastMouse;

			diff.y *= -1;
			data.frame->offsetX += diff.x;
			data.frame->offsetY += diff.y;
			boxButton->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.frame->offsetX - (data.frame->texWidth * data.frame->blendOptions.scaleX) / 2)), "&.h / 2 + " + std::to_string(data.frame->offsetY + 300));
			gui.get<tgui::EditBox>("Offset")->setText("(" + std::to_string(data.frame->offsetX) + "," + std::to_string(data.frame->offsetY) + ")");
			arrangeButtons(&*object);*/
		} else {
			Vector2i diff{mouseX, mouseY};

			diff -= lastMouse;
			selectedBox->left += diff.x;
			selectedBox->right += diff.x;
			selectedBox->down += diff.y;
			selectedBox->up += diff.y;
			boxButton->setPosition("&.w / 2 + " + std::to_string(selectedBox->left), "&.h / 2 + " + std::to_string(selectedBox->up + 300));
			arrangeButtons(&*object);
		}
		lastMouse = {mouseX, mouseY};
		return;
	}
	if (dragLeft) {
		auto diff = mouseX - lastMouse.x;

		if (spriteSelected) {/*
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.frame->texWidth * data.frame->blendOptions.scaleX) - diff < 10) {
				data.frame->offsetX += data.frame->texWidth * data.frame->blendOptions.scaleX - 10;
				lastMouse.x += data.frame->texWidth * data.frame->blendOptions.scaleX - 10;
				data.frame->blendOptions.scaleX = data.frame->texWidth / 10;
			} else {
				data.frame->offsetX += diff;
				bbb = !bbb;
				bbb &= diff % 2;
				data.frame->size.x -= diff + copysign(bbb, diff);
				lastMouse.x = mouseX;
			}*/
		} else {
			/*if (static_cast<int>(selectedBox->size.x) - diff < 10) {
				selectedBox->pos.x += selectedBox->size.x - 10;
				lastMouse.x += selectedBox->size.x - 10;
				selectedBox->size.x = 10;
			} else {*/
			selectedBox->left += diff;
			//selectedBox->size.x -= diff;
			lastMouse.x = mouseX;
			//}
		}
	} else if (dragRight) {
		auto diff = mouseX - lastMouse.x;

		if (spriteSelected) {/*
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.frame->size.x) + diff < 10) {
				lastMouse.x -= data.frame->size.x - 10;
				data.frame->offset.x += (data.frame->size.x - 10) / 2;
				data.frame->size.x = 10;
			} else {
				data.frame->size.x += diff;
				bbb = !bbb;
				bbb &= diff % 2;
				data.frame->offset.x += diff / 2 + copysign(bbb, diff);
				lastMouse.x = mouseX;
			}*/
		} else {
			/*if (static_cast<int>(selectedBox->size.x) + diff < 10) {
				lastMouse.x -= selectedBox->size.x - 10;
				selectedBox->size.x = 10;
			} else {*/
				//selectedBox->size.x += diff;
				selectedBox->right += diff;
				lastMouse.x = mouseX;
			//}
		}
	}
	if (dragUp) {
		auto diff = mouseY - lastMouse.y;

		if (spriteSelected) {/*
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.frame->size.y) - diff < 10) {
				lastMouse.y += data.frame->size.y - 10;
				data.frame->size.y = 10;
			} else {
				data.frame->size.y -= diff;
				lastMouse.y = mouseY;
			}*/
		} else {
			/*if (static_cast<int>(selectedBox->size.y) - diff < 10) {
				selectedBox->pos.y += selectedBox->size.y - 10;
				lastMouse.y += selectedBox->size.y - 10;
				selectedBox->size.y = 10;
			} else {*/
				//selectedBox->pos.y += diff;
				//selectedBox->size.y -= diff;
				selectedBox->up += diff;
				lastMouse.y = mouseY;
			//}
		}
	} else if (dragDown) {
		auto diff = mouseY - lastMouse.y;

		if (spriteSelected) {/*
			auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

			if (static_cast<int>(data.frame->size.y) + diff < 10) {
				data.frame->offset.y -= data.frame->size.y - 10;
				lastMouse.y -= data.frame->size.y - 10;
				data.frame->size.y = 10;
			} else {
				data.frame->offset.y -= diff;
				data.frame->size.y += diff;
				lastMouse.y = mouseY;
			}*/
		} else {
			/*if (static_cast<int>(selectedBox->size.y) + diff < 10) {
				lastMouse.y -= selectedBox->size.y - 10;
				selectedBox->size.y = 10;
			} else {*/
				//selectedBox->size.y += diff;
				selectedBox->down += diff;
				lastMouse.y = mouseY;
			//}
		}
	}
	if (spriteSelected) {/*
		auto &data = object->_moves.at(object->_action)[object->_actionBlock][object->_animation];

		boxButton->setPosition("&.w / 2 + " + std::to_string(static_cast<int>(data.frame->offset.x - data.frame->size.x / 2)), "&.h / 2 + " + std::to_string(data.frame->offset.y + 300));
		boxButton->setSize(data.frame->size.x, data.frame->size.y);
		gui.get<tgui::EditBox>("Offset")->setText("(" + std::to_string(data.frame->offset.x) + "," + std::to_string(data.frame->offset.y) + ")");
		gui.get<tgui::EditBox>("Size")->setText("(" + std::to_string(data.frame->size.x) + "," + std::to_string(data.frame->size.y) + ")");
		arrangeButtons(&*object);*/
	} else {
		boxButton->setPosition("&.w / 2 + " + std::to_string(selectedBox->left), "&.h / 2 + " + std::to_string(selectedBox->up + 300));
		boxButton->setSize(selectedBox->right - selectedBox->left, selectedBox->down - selectedBox->up);
		arrangeButtons(&*object);
	}
}

bool	isEditBoxSelected(const tgui::Panel::Ptr &panel, const tgui::ChildWindow::Ptr &window)
{
	auto &widgets = panel->getWidgets();
	auto cb = [](const tgui::Widget::Ptr &widget){
		auto box = widget->cast<tgui::EditBox>();
		auto pan = widget->cast<tgui::Panel>();

		return (pan && isEditBoxSelected(pan, nullptr)) || (box && box->isFocused());
	};

	return std::any_of(widgets.begin(), widgets.end(), cb) || (window && std::any_of(window->getWidgets().begin(), window->getWidgets().end(), cb));
}

void	updatePalsBulk(tgui::ChildWindow::Ptr window, const std::string &clipboard, std::unique_ptr<EditableObject> &object)
{
	auto pal = window->get<tgui::ComboBox>("Palettes");
	auto col = window->get<tgui::ScrollablePanel>("Colors");
	size_t pos = 0;
	std::string s = clipboard;
	std::string token;
	std::vector<std::string> tokens;

	for (pos = s.find('\n'); pos != std::string::npos; pos = s.find('\n')) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + 1);
	}
	tokens.push_back(s);

	for (auto &str : tokens) {
		try {
			auto button = col->get<tgui::Button>("Color" + std::to_string(selectedColor));
			tgui::Color color{str};
			sf::Color bufferColor{
				color.getRed(),
				color.getGreen(),
				color.getBlue()
			};


			if (!button)
				return;
			if (!editSession.palette)
				return;

			auto renderer = button->getRenderer();
			auto &newpal = ((uint16_t *) editSession.palette->data)[selectedColor];

			newpal = 0;
			newpal |= (bufferColor.r >> 3 & 0x1F) << 10;
			newpal |= (bufferColor.g >> 3 & 0x1F) << 5;
			newpal |= (bufferColor.b >> 3 & 0x1F) << 0;
			if (selectedColor)
				newpal |= 0x8000;
			renderer->setBackgroundColor(bufferColor);
			renderer->setBackgroundColorDown(bufferColor);
			renderer->setBackgroundColorDisabled(bufferColor);
			renderer->setBackgroundColorFocused(bufferColor);
			renderer->setBackgroundColorHover(bufferColor);
		} catch (std::exception &e) {
			game->logger.error(e.what());
		}
		selectedColor++;
		if (selectedColor == 256) {
			selectedColor = 255;
			break;
		}
	}
	// TODO: Properly update the palette editor UI to reflect which color is now selected
	game->textureMgr.invalidatePalette(editSession.palName);
	for (auto &action : object->_moves)
		for (auto &block : action.second)
			for (auto &anim : block)
				anim.setPalette(*editSession.palette, editSession.palName);
}

void	handleKeyPress(sf::Event::KeyEvent event, std::unique_ptr<EditableObject> &object, tgui::Gui &gui)
{
	auto bar = gui.get<tgui::MenuBar>("main_bar");
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto boxes = gui.get<tgui::Panel>("Boxes");
	auto window = gui.get<tgui::ChildWindow>("PaletteEditor");

	if (event.code == sf::Keyboard::S) {
		if (event.control)
			saveFramedataToFileCallback(object);
	}
	if (event.code == sf::Keyboard::Q)
		quitCallback();
	if (event.code == sf::Keyboard::N && event.control)
		newFileCallback(object, bar, gui);
	if (event.code == sf::Keyboard::O && event.control && event.shift)
		openFramedataFromFile(object, bar, gui);
	else if (event.code == sf::Keyboard::O && event.control)
		openFramedataFromPackage(object, bar, gui);

	if (object) {
		if (event.code == sf::Keyboard::F) {
			if (event.control && event.shift)
				newEndFrameCallback(gui, object, panel);
			else if (event.control)
				newFrameCallback(gui, object, panel);
		}
		if (event.code == sf::Keyboard::B && event.control)
			newAnimBlockCallback(gui, object, panel);
		if (event.code == sf::Keyboard::H) {
			if (event.control && event.shift)
				newHitBoxCallback(object, boxes);
			else if (event.control)
				newHurtBoxCallback(object, boxes);
			else
				switchHitboxes(object, boxes);
		}

		if (event.code == sf::Keyboard::Delete) {
			if (event.control && event.shift)
				removeFrameCallback(object, boxes);
			else if (event.control)
				removeActionCallback(gui, object);
			else if (event.shift)
				removeAnimationBlockCallback(object, panel);
			else
				removeBoxCallback(boxes, object, panel);
		}

		if (event.code == sf::Keyboard::C) {
			if (event.control) {
				if (!isEditBoxSelected(panel, window))
					sf::Clipboard::setString(object->_moves[object->_action][object->_actionBlock][object->_animation].saveData().dump());
			}
		}

		if (event.code == sf::Keyboard::V) {
			std::string clipboard = sf::Clipboard::getString().toAnsiString();

			if (clipboard.front() == '{') {
				if (event.control && event.shift) {
					if (isEditBoxSelected(panel, window))
						return;
					try {
						object->_moves[object->_action][object->_actionBlock][object->_animation].loadBoxes(
							nlohmann::json::parse(clipboard)
						);
						refreshFrameDataPanel(panel, boxes, object);
					} catch (std::exception &e) {
						SpiralOfFate::game->logger.info("Clipboard contains invalid data: " + std::string(e.what()));
					}
				} else if (event.control && event.alt) {
					if (isEditBoxSelected(panel, window))
						return;
					try {
						object->_moves[object->_action][object->_actionBlock][object->_animation].loadSpriteInfo(
							nlohmann::json::parse(clipboard)
						);
						refreshFrameDataPanel(panel, boxes, object);
					} catch (std::exception &e) {
						SpiralOfFate::game->logger.info("Clipboard contains invalid data: " + std::string(e.what()));
					}
				} else if (event.control) {
					if (isEditBoxSelected(panel, window))
						return;
					try {
						object->_moves[object->_action][object->_actionBlock][object->_animation].loadData(
							nlohmann::json::parse(clipboard)
						);
						refreshFrameDataPanel(panel, boxes, object);
					} catch (std::exception &e) {
						SpiralOfFate::game->logger.info("Clipboard contains invalid data: " + std::string(e.what()));
					}
				}
			} else if (clipboard.front() == '#') {
				// TODO: Disable if an editbox is selected
				if (event.control && window)
					updatePalsBulk(window, clipboard, object);
			} else
				game->logger.error("Invalid format: " + clipboard);
		}

		static int offset = 0;

		if (event.code == sf::Keyboard::Down) {
			auto &data = object->_moves[object->_action][object->_actionBlock][object->_animation];

			for (auto &b : data.frame->cBoxes) {
				b.up   += offset;
				b.down += offset;
			}
			for (auto &b : data.frame->hBoxes) {
				b.up   += offset;
				b.down += offset;
			}
			for (auto &b : data.frame->aBoxes) {
				b.up   += offset;
				b.down += offset;
			}
			if (data.frame->hasBlendOptions())
				data.frame->offsetY -= offset / (data.frame->blendOptions.scaleY / 100);
			else
				data.frame->offsetY -= offset;
			refreshBoxes(boxes, data, object);
		}
		if (event.code == sf::Keyboard::Up) {
			auto &data = object->_moves[object->_action][object->_actionBlock][object->_animation];

			for (auto &b : data.frame->cBoxes) {
				b.up   -= offset;
				b.down -= offset;
			}
			for (auto &b : data.frame->hBoxes) {
				b.up   -= offset;
				b.down -= offset;
			}
			for (auto &b : data.frame->aBoxes) {
				b.up   -= offset;
				b.down -= offset;
			}
			if (data.frame->hasBlendOptions())
				data.frame->offsetY += offset / (data.frame->blendOptions.scaleY / 100);
			else
				data.frame->offsetY += offset;
			refreshBoxes(boxes, data, object);
		}
		if (event.code == sf::Keyboard::P) {
			std::cin >> offset;
		}
	}
}

void	loadSettings()
{
	game->logger.debug("Loading settings");

	std::ifstream file("assets/settings.json");
	nlohmann::json json;

	if (file.is_open()) {
		file >> json;
		file.close();

		swr = json["swr"];
		soku = json["soku"];
		soku2 = json["soku2"];
		extra = json.contains("extra") ? json["extra"].get<std::vector<std::string>>() : std::vector<std::string>{};
		bgColor = json.contains("bgColor") ? sf::Color(json["bgColor"]) : sf::Color::Black;
		bgColor.a = 255;
	}
	verifySettings();
	loadPackages();
}

std::filesystem::path cwd;

void	renderTopLayer(EditableObject &object)
{
	sf::RectangleShape rect;
	auto &data = object._moves.at(object._action)[object._actionBlock][object._animation];
	auto translate = object.displayScaled ? object.translate : SpiralOfFate::Vector2f{0, 0};
	auto s = object.displayScaled ? object.scale : 1.f;
	SpiralOfFate::Vector2f scale;
	auto bounds = SpiralOfFate::Vector2f{
		static_cast<float>(data.frame->texWidth),
		static_cast<float>(data.frame->texHeight)
	};
	auto texBounds = sf::IntRect{
		data.frame->texOffsetX,
		data.frame->texOffsetY,
		data.frame->texWidth,
		data.frame->texHeight
	};
	auto result = SpiralOfFate::Vector2f{
		static_cast<float>(-data.frame->offsetX) * s,
		static_cast<float>(-data.frame->offsetY) * s
	};

	if (data.frame->renderGroup == 0) {
		scale.x = 2;
		scale.y = 2;
	} else if (data.frame->renderGroup == 2) {
		scale.x = data.frame->blendOptions.scaleX / 100.f;
		scale.y = data.frame->blendOptions.scaleY / 100.f;
		result.x *= scale.x;
		result.y *= scale.y;
	} else {
		scale.x = 1;
		scale.y = 1;
	}
	scale.x *= s;
	scale.y *= s;
	if (data.frame->blendOptions.flipHorz) {
		texBounds.left += texBounds.width;
		texBounds.width *= -1;
	}
	if (data.frame->blendOptions.flipVert) {
		texBounds.top += texBounds.height;
		texBounds.height *= -1;
	}
	//result.y *= -1;
	//result += SpiralOfFate::Vector2f{
	//	size.x / -2.f,
	//	-static_cast<float>(size.y)
	//};
	result += SpiralOfFate::Vector2f{
		data.frame->texWidth * scale.x / 2,
		data.frame->texHeight * scale.y / 2
	};
	result += translate;
	selectedColorMaskSprite.setOrigin(bounds / 2.f);
	selectedColorMaskSprite.setRotation(data.frame->blendOptions.angle);
	selectedColorMaskSprite.setPosition(result);
	selectedColorMaskSprite.setScale(scale);
	selectedColorMaskSprite.setTextureRect(texBounds);
	SpiralOfFate::game->screen->draw(selectedColorMaskSprite);
}

Vector2f mousePosToImgPos(EditableObject &object, Vector2i mouse, const FrameData &data, tgui::Panel::Ptr panel)
{
	auto translate = object.displayScaled ? object.translate : SpiralOfFate::Vector2f{0, 0};
	auto s = object.displayScaled ? object.scale : 1.f;
	SpiralOfFate::Vector2f scale;
	auto bounds = SpiralOfFate::Vector2f{
		static_cast<float>(data.frame->texWidth),
		static_cast<float>(data.frame->texHeight)
	};
	auto texBounds = sf::IntRect{
		data.frame->texOffsetX,
		data.frame->texOffsetY,
		data.frame->texWidth,
		data.frame->texHeight
	};
	auto result = SpiralOfFate::Vector2f{
		static_cast<float>(-data.frame->offsetX) * s,
		static_cast<float>(-data.frame->offsetY) * s
	};
	auto view = game->screen->getView();

	if (data.frame->renderGroup == 0) {
		scale.x = 2;
		scale.y = 2;
	} else if (data.frame->renderGroup == 2) {
		scale.x = data.frame->blendOptions.scaleX / 100.f;
		scale.y = data.frame->blendOptions.scaleY / 100.f;
		result.x *= scale.x;
		result.y *= scale.y;
	} else {
		scale.x = 1;
		scale.y = 1;
	}
	scale.x *= s;
	scale.y *= s;

	auto imgPos = result + translate + object._position + SpiralOfFate::Vector2f{
		(game->screen->getSize().x / 2 - panel->getSize().x / 2),
		static_cast<float>(game->screen->getSize().y / 2 + 300)
	};

	return (mouse - imgPos) / scale;
}

bool	selectColor(tgui::Gui &gui, std::unique_ptr<EditableObject> &obj, int mouseX, int mouseY, tgui::Panel::Ptr panel)
{
	if (!obj)
		return false;

	auto &object = *obj;
	auto &data = object._moves.at(object._action)[object._actionBlock][object._animation];
	const ShadyCore::Image *_img;

	try {
		_img = &game->textureMgr.getUnderlyingImage("data/character/" + editSession.chr + "/" + data._schema->images.at(data.frame->imageIndex).name);
	} catch (std::out_of_range &e) {
		game->logger.debug("selectColor: Invalid image: " + std::string(e.what()));
		return false;
	}

	auto &img = *_img;

	if (img.bitsPerPixel != 8)
		return false;

	sf::RectangleShape rect;
	auto pos = mousePosToImgPos(object, {mouseX, mouseY}, data, panel);
	auto bounds = SpiralOfFate::Vector2f{
		static_cast<float>(data.frame->texWidth),
		static_cast<float>(data.frame->texHeight)
	};

	if (pos.y < 0 || pos.y > bounds.y || pos.x < 0 || pos.x > bounds.x)
		return false;

	int i = img.raw[static_cast<int>(pos.y) * img.paddedWidth + static_cast<int>(pos.x)];
	int old = selectedColor;
	tgui::ChildWindow::Ptr window = gui.get<tgui::ChildWindow>("PaletteEditor");

	if (!i)
		return false;
	selectedColor = i;
	if (!window)
		return true;

	auto pal = window->get<tgui::ComboBox>("Palettes");
	auto col = window->get<tgui::ScrollablePanel>("Colors");
	auto light = window->get<tgui::Slider>("Lightness");
	auto satHuePic = window->get<tgui::Picture>("SatHue");
	auto cross = window->get<tgui::Picture>("Cross");
	auto preview = window->get<tgui::TextBox>("Preview");
	auto hue = window->get<tgui::EditBox>("Hue");
	auto saturation = window->get<tgui::EditBox>("Sat");
	auto lightness = window->get<tgui::EditBox>("Light");
	auto edit = window->get<tgui::EditBox>("Edit");
	auto label = window->get<tgui::Label>("ColorN");
	auto button = col->get<tgui::Button>("Color" + std::to_string(i));
	char buffer[8];
	auto val = ((uint16_t*)editSession.palette->data)[i];
	sf::Color color{
		static_cast<sf::Uint8>((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12),
		static_cast<sf::Uint8>((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7),
		static_cast<sf::Uint8>((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2),
		static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
	};
	Utils::HSLColor c = Utils::RGBtoHSL(color);
	auto pos2 = satHuePic->getPosition();
	auto size = cross->getSize();

	label->setText("Color #" + std::to_string(i));
	col->get<tgui::Button>("Color" + std::to_string(old))->setFocused(false);
	sprintf(buffer, "#%02X%02X%02X", color.r, color.g, color.b);
	light->onValueChange.setEnabled(false);
	light->setValue(c.l);
	light->onValueChange.setEnabled(true);
	hue->setText(std::to_string(c.h));
	saturation->setText(std::to_string(c.s));
	lightness->setText(std::to_string(c.l));
	cross->setPosition({
		(c.h * 200 / 240) + pos2.x - size.x / 2,
		((240 - c.s) * 200 / 240) + pos2.y - size.y / 2
	});
	edit->setText(buffer);
	preview->getRenderer()->setBackgroundColor({buffer});

	sf::Image image;
	sf::Texture texture;

	image.create(light->getSize().x, light->getSize().y);

	auto s = image.getSize();

	for (unsigned y = 0; y < s.y; y++) {
		c.l = 240 - (240 * y / s.y);

		auto rgb = HSLtoRGB(c);
		sf::Color sf{rgb.r, rgb.g, rgb.b, 255};

		for (unsigned x = 0; x < s.x; x++)
			image.setPixel(x, y, sf);
	}
	texture.loadFromImage(image);
	light->getRenderer()->setTextureTrack({texture});

	button->setFocused(true);
	return true;
}

void 	hoverImagePixel(std::unique_ptr<EditableObject> &obj, int mouseX, int mouseY, tgui::Panel::Ptr panel)
{
	if (!obj)
		return;

	const ShadyCore::Image *_img;
	auto &object = *obj;
	auto &data = object._moves.at(object._action)[object._actionBlock][object._animation];

	try {
		_img = &game->textureMgr.getUnderlyingImage("data/character/" + editSession.chr + "/" + data._schema->images.at(data.frame->imageIndex).name);
	} catch (std::out_of_range &e) {
		game->logger.debug("hoverImagePixel: Invalid image: " + std::string(e.what()));
		return;
	}

	auto &img = *_img;

	if (img.bitsPerPixel != 8) {
		game->logger.debug("Image is not 8 bits per pixel: " + std::to_string(img.bitsPerPixel));
		hovering = false;
		return;
	}

	sf::RectangleShape rect;
	auto pos = mousePosToImgPos(object, {mouseX, mouseY}, data, panel);
	auto bounds = SpiralOfFate::Vector2f{
		static_cast<float>(data.frame->texWidth),
		static_cast<float>(data.frame->texHeight)
	};

	if (pos.y < 0 || pos.y > bounds.y || pos.x < 0 || pos.x > bounds.x) {
		hovering = false;
		return;
	}

	int i = img.raw[static_cast<int>(pos.y) * img.paddedWidth + static_cast<int>(pos.x)];

	if (!i) {
		hovering = false;
		return;
	}

	auto val = ((uint16_t*)editSession.palette->data)[i];
	sf::Color color{
		static_cast<sf::Uint8>(~((val & 0b0111110000000000) >> 7 | (val & 0b0111000000000000) >> 12)),
		static_cast<sf::Uint8>(~((val & 0b0000001111100000) >> 2 | (val & 0b0000001110000000) >> 7)),
		static_cast<sf::Uint8>(~((val & 0b0000000000011111) << 3 | (val & 0b0000000000011100) >> 2)),
		static_cast<sf::Uint8>((val & 0x8000) ? 255 : 0)
	};

	if (img.bitsPerPixel != 8)
		return;
	hovering = true;
	selectedColorMaskImage.create(img.width, img.height);
	for (unsigned y = 0; y < img.height; y++)
		for (unsigned x = 0; x < img.width; x++)
			if (i && img.raw[y * img.paddedWidth + x] == i)
				selectedColorMaskImage.setPixel(x, y, color);
			else
				selectedColorMaskImage.setPixel(x, y, sf::Color::Transparent);
	selectedColorMaskTexture.loadFromImage(selectedColorMaskImage);
	selectedColorMaskSprite.setTexture(selectedColorMaskTexture, true);
}

bool	onWidget(tgui::Gui &gui, int x, int y)
{
	for (auto &widget : gui.getWidgets())
		if (
			widget->isVisible() && widget->isEnabled() &&
			widget->getPosition().x <= x && widget->getPosition().x + widget->getSize().x >= x &&
			widget->getPosition().y <= y && widget->getPosition().y + widget->getSize().y >= y
		)
			return true;
	return false;
}

void	run()
{
	loadSettings();
	game->screen = std::make_unique<Screen>("Frame data editor version " VERSION_STR);

	std::unique_ptr<EditableObject> object;
	tgui::Gui gui{*game->screen};
	sf::Image icon;
	sf::Event event;
	/*auto current = game->characterPaths.begin();
	auto startExport = [&object, &gui](bool gif, bool cb, bool hurtb, bool hitb, sf::Color bgColor){
		gui.get<tgui::Panel>("Panel1")->get<tgui::SpinButton>("Speed")->setValue(0);

		auto panel = tgui::Panel::create({"100%", "100%"});

		panel->getRenderer()->setBackgroundColor({0, 0, 0, 75});
		gui.add(panel);

		auto window = tgui::ChildWindow::create("Exporting...", tgui::ChildWindow::TitleButton::None);

		window->setSize(360, 90);
		window->setPosition("(&.w - w) / 2", "(&.h - h) / 2");
		gui.add(window, "LoadingWindow");

		window->setFocused(true);

		const bool tabUsageEnabled = gui.isTabKeyUsageEnabled();
		auto closeWindow = [&gui, window, panel, tabUsageEnabled]{
			gui.remove(window);
			gui.remove(panel);
			gui.setTabKeyUsageEnabled(tabUsageEnabled);
		};
		unsigned x = 0;

		for (auto &[id, act]: object->_moves)
			for (const auto &i : act)
				x += i.size();
		Utils::setRenderer(window);
		window->loadWidgetsFromFile("assets/gui/loading.gui");

		auto bar = window->get<tgui::ProgressBar>("ProgressBar");
		auto progress = window->get<tgui::Label>("Progress");
		auto label = window->get<tgui::Label>("Label");
		auto doIt = [&object, cb, hurtb, hitb, bgColor, bar, progress, x, label] {
			auto anim = object->_animation;
			auto block = object->_actionBlock;
			auto oldAction = object->_action;
			unsigned current = 0;

			for (auto &[id, act]: object->_moves) {
				object->_action = id;

				auto name = actionNames.find(static_cast<SokuLib::Action>(object->_action));
				auto action = name == actionNames.end() ? "Action #" + std::to_string(object->_action) : name->second;

				for (unsigned i = 0; i < act.size(); i++) {
					auto &blk = act[i];

					object->_actionBlock = i;
					for (unsigned j = 0; j < blk.size(); j++) {
						uiTasks.emplace_back([&object, label, action]{
							label->setText(action + "  Block " + std::to_string(object->_actionBlock) + "  Animation " + std::to_string(object->_animation));
						});

						object->_animation = j;
						exportFrameToImage(object, cb, hurtb, hitb, bgColor);
						current++;

						uiTasks.emplace_back([current, bar, progress, x]{
							bar->setValue(current);
							progress->setText(std::to_string(current) + "/" + std::to_string(x));
						});
					}
				}
			}
			object->_actionBlock = block;
			object->_animation = anim;
			object->_action = oldAction;
		};

		bar->setMaximum(x);
		progress->setText("0/" + std::to_string(x));
		label->setText("Exporting data....");
		std::thread{[doIt, closeWindow]{
			doIt();
			uiTasks.emplace_back([closeWindow] {
				closeWindow();
			});
		}}.detach();
	};
	auto lff = [&object, &gui](const std::string &path, const std::string &character){
		auto bar = gui.get<tgui::MenuBar>("main_bar");

		editSession.palName = "data/character/" + character + "/palette000.pal";

		//auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});
		auto pal = game->characterPaths[character].palettes.find(editSession.palName);
		std::ifstream stream{path, std::fstream::binary};
		std::string ext = path.substr(path.find_last_of('.') + 1);

		game->logger.info("Loading framedata from file: " + path);
		if (stream.fail()) {
			Utils::dispMsg("Loading error", "Could not find file " + path + ": " + strerror(errno), MB_ICONERROR);
			return;
		}
		if (pal == game->characterPaths[character].palettes.end()) {
			Utils::dispMsg("Loading error", "Could not find palette " + editSession.palName, MB_ICONERROR);
			return;
		}

		editSession.chr = character;
		editSession.palName = pal->first;
		editSession.palette = &pal->second;
		tempSchema.destroy();
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_SCHEMA, ext == "xml" ? ShadyCore::FileType::SCHEMA_XML : ShadyCore::FileType::SCHEMA_GAME_PATTERN})(&tempSchema, stream);
		try {
			object.reset();
			object = std::make_unique<EditableObject>(editSession.chr, tempSchema, *editSession.palette, editSession.palName);
			loadedPath = path;
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, true);
			bar->setMenuEnabled({"Remove"}, true);
			bar->setMenuEnabled({"Misc"}, true);
			bar->setMenuItemEnabled({"File", "Export..."}, true);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, true);
		} catch (std::exception &e) {
			Utils::dispMsg("Error", e.what(), MB_ICONERROR);
			loadedPath = path;
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, false);
			bar->setMenuEnabled({"Remove"}, false);
			bar->setMenuEnabled({"Misc"}, false);
			bar->setMenuItemEnabled({"File", "Export..."}, false);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);
		}
		displayPalEditor(object, gui);
	};
	auto lfp = [&object, &gui](const std::string &character){
		auto bar = gui.get<tgui::MenuBar>("main_bar");

		editSession.palName = "data/character/" + character + "/palette000.pal";

		//auto path = Utils::openFileDialog("Open framedata", "assets", {{".*\\.json", "Frame data file"}});
		std::string path = "data/character/" + character + "/" + character + ".pat";
		auto pal = game->characterPaths[character].palettes.find(editSession.palName);
		auto entry = game->package.find(path, ShadyCore::FileType::TYPE_SCHEMA);

		game->logger.info("Loading framedata from package: " + path);
		if (entry == game->package.end()) {
			Utils::dispMsg("Loading error", "Could not find file " + path, MB_ICONERROR);
			return;
		}
		if (pal == game->characterPaths[character].palettes.end()) {
			Utils::dispMsg("Loading error", "Could not find palette " + editSession.palName, MB_ICONERROR);
			return;
		}

		editSession.chr = character;
		editSession.palName = pal->first;
		editSession.palette = &pal->second;
		tempSchema.destroy();
		ShadyCore::getResourceReader({ShadyCore::FileType::TYPE_SCHEMA, ShadyCore::FileType::SCHEMA_GAME_PATTERN})(&tempSchema, entry.open());
		entry.close();
		try {
			object.reset();
			object = std::make_unique<EditableObject>(editSession.chr, tempSchema, *editSession.palette, editSession.palName);
			loadedPath = path;
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, true);
			bar->setMenuEnabled({"Remove"}, true);
			bar->setMenuEnabled({"Misc"}, true);
			bar->setMenuItemEnabled({"File", "Export..."}, true);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, true);
		} catch (std::exception &e) {
			Utils::dispMsg("Error", e.what(), MB_ICONERROR);
			loadedPath = path;
			refreshRightPanel(gui, object);
			bar->setMenuEnabled({"New"}, false);
			bar->setMenuEnabled({"Remove"}, false);
			bar->setMenuEnabled({"Misc"}, false);
			bar->setMenuItemEnabled({"File", "Export..."}, false);
			bar->setMenuItemEnabled({"File", "Import...", "palette..."}, false);
		}
		displayPalEditor(object, gui);
	};*/

	game->logger.debug("Load icon");
	if (icon.loadFromFile("assets/editorIcon.png"))
		game->screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

	game->logger.debug("Load widgets");
	gui.loadWidgetsFromFile("assets/gui/editor.gui");
	game->logger.debug("Done");

	bool dragging = false;
	auto panel = gui.get<tgui::Panel>("Panel1");
	auto animpanel = gui.get<tgui::Panel>("AnimPanel");
	auto block = panel->get<tgui::SpinButton>("Block");
	auto progress = panel->get<tgui::Slider>("Progress");
	sf::View view;
	sf::View guiView{
		{
			0, 0,
			static_cast<float>(game->screen->getSize().x),
			static_cast<float>(game->screen->getSize().y)
		}
	};

	placeGuiHooks(gui, object);
	view.setCenter(panel->getSize().x / 2, -300);
	view.setSize(game->screen->getSize().x, game->screen->getSize().y);
	game->screen->setView(view);
	gui.setView(guiView);
	/*while (current->first != "nue")
		current++;
	current++;*/
	while (game->screen->isOpen()) {
		/*if (current != game->characterPaths.end() && !gui.get<tgui::ChildWindow>("LoadingWindow")) {
			if (std::filesystem::exists("exported/" + current->first + ".pat"))
				lff("exported/" + current->first + ".pat", current->first);
			else
				lfp(current->first);
			startExport(false, true, true, true, sf::Color::Transparent);
			object->render(true);
			current++;
		}*/
		if (!hovering)
			timer++;
		game->screen->clear(bgColor);
		if (object) {
			if (timer >= updateTimer || updateAnyway) {
				auto b = object->_actionBlock;
				auto a = object->_animation;

				object->update();
				updateAnyway = false;
				if (b != object->_actionBlock)
					block->setValue(object->_actionBlock);
				else if (a != object->_animation)
					progress->setValue(object->_animation);
				timer -= updateTimer;
				if (overlayData.play) {
					auto window = gui.get<tgui::ChildWindow>("Overlay move");

					b = overlayData.object->_actionBlock;
					a = overlayData.object->_animation;
					overlayData.object->update();
					if (window) {
						auto block2 = window->get<tgui::SpinButton>("Block");
						auto frame = window->get<tgui::Slider>("Frame");

						if (b != overlayData.object->_actionBlock)
							block2->setValue(overlayData.object->_actionBlock);
						else if (a != overlayData.object->_animation)
							frame->setValue(overlayData.object->_animation);
					}
				}
			}
			if (overlayData.display && !overlayData.front)
				overlayData.object->render();
			object->render(gui.get<tgui::ChildWindow>("LoadingWindow") == nullptr);
			if (overlayData.display && overlayData.front)
				overlayData.object->render();
			if (hovering)
				renderTopLayer(*object);
		}

		std::optional<sf::Vector2i> drag;

		while (game->screen->pollEvent(event)) {
			if (cwd != std::filesystem::current_path()) {
				game->logger.warn("Current directory changed " + std::filesystem::current_path().string());
				std::filesystem::current_path(cwd);
			}

			auto handled = gui.handleEvent(event);

			if (event.type == sf::Event::Closed) {
				quitRequest = true;
				continue;
			}
			if (event.type == sf::Event::KeyPressed) {
				if (
					!event.key.control ||
					event.key.code == sf::Keyboard::C ||
					event.key.code == sf::Keyboard::V
				) {
					if (panel->get<tgui::EditBox>("Action")->isFocused())
						goto loopEnd;
					for (auto &widget: animpanel->getWidgets()) {
						auto box = widget->cast<tgui::EditBox>();

						if (!box)
							continue;
						if (box->isFocused())
							goto loopEnd;
					}
				}
				handleKeyPress(event.key, object, gui);
			loopEnd:
				continue;
			}
			if (!handled && event.type == sf::Event::MouseWheelScrolled && object && !displayHitboxes) {
				object->scale *= 1 + std::copysign(0.1f, event.mouseWheelScroll.delta);
				if (object->scale < 1)
					object->scale = 1;
				if (object->scale > 4)
					object->scale = 4;
				overlayData.object->scale = object->scale;
				continue;
			}
			if (event.type == sf::Event::Resized) {
				guiView.setSize(event.size.width, event.size.height);
				guiView.setCenter(event.size.width / 2, event.size.height / 2);
				gui.setView(guiView);

				view.setCenter(panel->getSize().x / 2, -300);
				view.setSize(event.size.width, event.size.height);
				game->screen->setView(view);
				continue;
			}
			if (dragging)
				dragging &= sf::Mouse::isButtonPressed(sf::Mouse::Left) ||
					sf::Mouse::isButtonPressed(sf::Mouse::Right) ||
					sf::Mouse::isButtonPressed(sf::Mouse::Middle) ||
					sf::Mouse::isButtonPressed(sf::Mouse::XButton1) ||
					sf::Mouse::isButtonPressed(sf::Mouse::XButton2);
			if (
				event.type == sf::Event::MouseButtonPressed && (
					(
						event.mouseButton.button == sf::Mouse::Left &&
						(
							handled ||
							onWidget(gui, event.mouseButton.x, event.mouseButton.y) ||
							!selectColor(gui, object, event.mouseButton.x, event.mouseButton.y, panel)
						)
					) || event.mouseButton.button != sf::Mouse::Left
				) && !dragging
			) {
				dragging = true;
				dragStart = false;
				mouseStart = {event.mouseButton.x, event.mouseButton.y};
				lastMouse = mouseStart;
				normalDrag = !handled;
			}
			dragUp &= dragging;
			dragDown &= dragging;
			dragLeft &= dragging;
			dragRight &= dragging;
			canDrag &= dragging;
			colorPickDrag &= dragging;
			if (event.type == sf::Event::MouseMoved && dragging) {
				if (canDrag)
					handleDrag(gui, object, event.mouseMove.x, event.mouseMove.y);
				else if (colorPickDrag)
					drag = {event.mouseMove.x, event.mouseMove.y};
				else if (!displayHitboxes && object && normalDrag && !handled) {
					object->translate += Vector2f{
						static_cast<float>(event.mouseMove.x - lastMouse.x),
						static_cast<float>(event.mouseMove.y - lastMouse.y)
					};
					overlayData.object->translate = object->translate + overlayData.offset;
					lastMouse = Vector2i{event.mouseMove.x, event.mouseMove.y};
				} else if (!handled)
					hoverImagePixel(object, event.mouseMove.x, event.mouseMove.y, panel);
			} else if (event.type == sf::Event::MouseMoved && !handled)
				hoverImagePixel(object, event.mouseMove.x, event.mouseMove.y, panel);
		}
		if (drag)
			handleColorPickDrag(gui, object, drag->x, drag->y);
		if (object)
			object->displayScaled = !displayHitboxes;
		while (!uiTasks.empty()) {
			uiTasks.front()();
			uiTasks.pop_front();
		}
		gui.draw();
		game->screen->display();
		if (quitRequest) {
			if (!object) {
				game->screen->close();
				continue;
			}
			quitRequest = false;
			if (gui.get<tgui::ChildWindow>("QuitConfirm"))
				continue;

			auto window = Utils::openWindowWithFocus(gui, 260, 90);

			window->loadWidgetsFromFile("assets/gui/quitConfirm.gui");
			window->get<tgui::Button>("Yes")->connect("Clicked", [&object](std::weak_ptr<tgui::ChildWindow> self){
				if (saveFramedataToFileCallback(object))
					game->screen->close();
			}, std::weak_ptr<tgui::ChildWindow>(window));
			window->get<tgui::Button>("No")->connect("Clicked", []{
				game->screen->close();
			});
			window->get<tgui::Button>("Cancel")->connect("Clicked", [](std::weak_ptr<tgui::ChildWindow> self){
				self.lock()->close();
			}, std::weak_ptr<tgui::ChildWindow>(window));
		}
	}
}

#ifdef _WIN32
std::string getLastError(int err = GetLastError())
{
	char *s = nullptr;
	std::string str;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, nullptr
	);
	str = s;
	LocalFree(s);
	return str;
}

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	char buf[MAX_PATH];
	char buf2[MAX_PATH];
	time_t timer;
	char timebuffer[56];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, 26, "%Y%m%d%H%M%S", tm_info);
	mkdir("crash_dumps", 0);
	sprintf(buf2, "crash_dumps/crash_%s.dmp", timebuffer);

	HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

		if (!win)
			sprintf(buf, "Fatal exception %x caught.\nMiniDumpWriteDump failed.\n%s: %s", ExPtr->ExceptionRecord->ExceptionCode, getLastError().c_str(), buf2);
		else
			sprintf(buf, "Fatal exception %x caught.\nMinidump created %s", ExPtr->ExceptionRecord->ExceptionCode, buf2);
		CloseHandle(hFile);
	} else
		sprintf(buf, "Fatal exception %x caught.\nCould not create file %s\n%s", ExPtr->ExceptionRecord->ExceptionCode, buf2, getLastError().c_str());
	game->logger.fatal(buf);
	game->logger.flush();
	Utils::dispMsg("Fatal error", buf, MB_ICONERROR, &*game->screen);
	exit(EXIT_FAILURE);
}
#endif

int	main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#endif
	cwd = std::filesystem::current_path();
#ifndef _DEBUG
	try {
#endif
		new Game("./editor.log");
		game->logger.info("Starting editor.");
		run();
		game->logger.info("Goodbye !");
#ifndef _DEBUG
	} catch (std::exception &e) {
		game->logger.fatal(e.what());
		Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, &*game->screen);
		delete game;
		return EXIT_FAILURE;
	}
#endif
	saveSettings();
	delete game;
	return EXIT_SUCCESS;
}