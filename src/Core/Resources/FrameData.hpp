//
// Created by andgel on 18/09/2021
//

#ifndef BATTLE_FRAMEDATA_HPP
#define BATTLE_FRAMEDATA_HPP


#include <map>
#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <SFML/Graphics/Rect.hpp>
#include "../Data/Vector.hpp"
#include "../Data/Color.hpp"
#include <resource.hpp>

namespace SpiralOfFate
{
	struct Box {
		Vector2i pos;
		Vector2u size;
		operator sf::IntRect() const noexcept;
	};

	union DefensiveFlags {
		unsigned flags;
		struct {
			bool invulnerable : 1;
			bool invulnerableArmor : 1;
			bool superarmor : 1;
			bool grabInvulnerable : 1;
			bool voidBlock : 1;
			bool spiritBlock : 1;
			bool matterBlock : 1;
			bool neutralBlock : 1;
			bool airborne : 1;
			bool canBlock : 1;
			bool highBlock : 1;
			bool lowBlock : 1;
			bool karaCancel : 1;
			bool resetRotation : 1;
			bool counterHit : 1;
			bool flash : 1;
			bool crouch : 1;
			bool projectileInvul : 1;
			bool projectile : 1;
			bool landCancel : 1;
			bool dashCancel : 1;
			bool resetSpeed : 1;
			bool neutralInvul : 1;
			bool matterInvul : 1;
			bool spiritInvul : 1;
			bool voidInvul : 1;
		};
	};

	union OffensiveFlags {
		unsigned flags;
		struct {
			bool grab : 1;
			bool airUnblockable : 1;
			bool unblockable : 1;
			bool voidElement : 1;
			bool spiritElement : 1;
			bool matterElement : 1;
			bool lowHit : 1;
			bool highHit : 1;
			bool autoHitPos : 1;
			bool canCounterHit : 1;
			bool hitSwitch : 1;
			bool cancelable : 1;
			bool jab : 1;
			bool resetHits : 1;
			bool resetSpeed : 1;
			bool restand : 1;
			bool super : 1;
			bool ultimate : 1;
			bool jumpCancelable : 1;
			bool transformCancelable : 1;
			bool unTransformCancelable : 1;
			bool dashCancelable : 1;
			bool backDashCancelable : 1;
			bool voidMana : 1;
			bool spiritMana : 1;
			bool matterMana : 1;
		};
	};

	class FrameData {
	private:
		bool _slave = false;
		std::string _pal;
		std::string _character;
		const ShadyCore::Palette *_palette;

	public:
		const ShadyCore::Schema *_schema;
		unsigned textureHandle = 0;
		unsigned hitSoundHandle = 0;
		bool needReload = false;
		ShadyCore::Schema::Sequence *parent;
		ShadyCore::Schema::Sequence::MoveFrame *frame;
		std::optional<unsigned> clone;

		FrameData(const FrameData &other);
		~FrameData();
		FrameData(const std::string &chr, const ShadyCore::Schema &schema, ShadyCore::Schema::Sequence &parent, ShadyCore::Schema::Sequence::MoveFrame &frame, const ShadyCore::Palette &palette, const std::string &palName);
		FrameData(const FrameData &other, ShadyCore::Schema::Sequence::MoveFrame &frame);
		FrameData &operator=(const FrameData &other);
		FrameData &operator=(FrameData &&other) noexcept;
		void setSlave(bool slave = true);
		void reloadTexture();
		void reloadSound();
		void setPalette(const ShadyCore::Palette &palette, const std::string &name);
		nlohmann::json saveBoxes() const;
		void loadBoxes(const nlohmann::json &json);

		static std::map<unsigned, std::vector<std::vector<FrameData>>> loadSchema(const std::string &chr, const ShadyCore::Schema &schema, const ShadyCore::Palette &palette, const std::string &palName);
	};
}


#endif //BATTLE_FRAMEDATA_HPP
