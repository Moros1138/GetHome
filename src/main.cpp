#include "olcPixelGameEngine3.h"
#include "utilities/olcUTIL3_Geometry2D.h"
#include "utilities/olcUTIL3_Animate2D.h"
#include "utilities/olcUTIL3_GameMode.h"

#include "tileson.hpp"

#define MOVE_SPEED 6.0f
#define TILE_SIZE 16

enum State {
	NONE,
	SPLASH,
	GAME,
	TELEPORT,
	GAMEOVER,
	END_GAME,
	CREDITS
};

class GetHome_Game : public olc::PixelGameEngine
{
public:
	GetHome_Game()
	{
		sAppName = "GetHome";
	}

public:
	bool OnUserCreate() override
	{
		CreateImageFromFile(imgCharacter, "assets/olcBTB_character.png");
		CreateImageFromFile(imgTileset, "assets/olcBTB_tileset1.png");
		CreateImage(imgHeadsUpDisplay, {110, 16});
		CreateImageFromFile(imgSplash, "assets/olcBTB_splash.png");
		CreateImageFromFile(imgCredits, "assets/olcBTB_credits.png");
		CreateImage(imgShadow, {32, 32});

		draw.SetTarget(imgHeadsUpDisplay);

		draw.Clear(olc::Colour::BLANK);
		draw.String({1, 1}, "Time", olc::Colour::BLACK);
		draw.String({0, 0}, "Time", olc::Colour::WHITE);
		
		draw.FilledRect({2, 12}, {100, 4}, olc::Colour::BLACK);
		draw.FilledRect({0, 10}, {100, 4}, olc::Colour::YELLOW);
		draw.Rect({0, 10}, {100, 4}, olc::Colour::WHITE);
		
		draw.SetTarget(imgShadow);
		draw.Clear(olc::Colour::BLANK);
		draw.FilledCircle(olc::vi2d{ draw.GetTargetSize().x / 2, draw.GetTargetSize().x - 6 }, 6, olc::Pixel(0, 0, 20, 128));
		
		draw.SetTarget(GetScreen());

		// define "up" sprite
		olc::utils::Animate2D::FrameSequence anim_fs_walk_up;
		anim_fs_walk_up.AddFrame(imgCharacter.region({   0, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({  32, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({  64, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({  96, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({ 128, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({ 160, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({ 192, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({ 224, 0 }, { 32, 32 }));
		anim_fs_walk_up.AddFrame(imgCharacter.region({ 256, 0 }, { 32, 32 }));

		// define "left" sprite
		olc::utils::Animate2D::FrameSequence anim_fs_walk_left;
		anim_fs_walk_left.AddFrame(imgCharacter.region({   0, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({  32, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({  64, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({  96, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({ 128, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({ 160, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({ 192, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({ 224, 32 }, { 32, 32 }));
		anim_fs_walk_left.AddFrame(imgCharacter.region({ 256, 32 }, { 32, 32 }));

		// define "down" sprite
		olc::utils::Animate2D::FrameSequence anim_fs_walk_down;
		anim_fs_walk_down.AddFrame(imgCharacter.region({   0, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({  32, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({  64, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({  96, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({ 128, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({ 160, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({ 192, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({ 224, 64 }, { 32, 32 }));
		anim_fs_walk_down.AddFrame(imgCharacter.region({ 256, 64 }, { 32, 32 }));

		// define "right" sprite
		olc::utils::Animate2D::FrameSequence anim_fs_walk_right;
		anim_fs_walk_right.AddFrame(imgCharacter.region({   0, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({  32, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({  64, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({  96, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({ 128, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({ 160, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({ 192, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({ 224, 96 }, { 32, 32 }));
		anim_fs_walk_right.AddFrame(imgCharacter.region({ 256, 96 }, { 32, 32 }));

		animPlayer.AddState(PlayerAnimationState::WALK_UP, anim_fs_walk_up);
		animPlayer.AddState(PlayerAnimationState::WALK_DOWN, anim_fs_walk_down);
		animPlayer.AddState(PlayerAnimationState::WALK_LEFT, anim_fs_walk_left);
		animPlayer.AddState(PlayerAnimationState::WALK_RIGHT, anim_fs_walk_right);
		
		game.playerAnimationState = PlayerAnimationState::WALK_DOWN;
		animPlayer.ChangeState(game.animstate, PlayerAnimationState::WALK_DOWN);

		tMap = tParser.parse("assets/outdoors.json");
		tTileset = tMap->getTileset("olcBTB_tileset1");
		lObjects = tMap->getLayer("objects");
		
		game.state = SPLASH;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if(game.state == State::NONE) return false;
		if(game.state == State::SPLASH) DoSplash(fElapsedTime);
		if(game.state == State::GAME) DoGame(fElapsedTime);
		if(game.state == State::TELEPORT) DoTeleport(fElapsedTime);
		if(game.state == State::GAMEOVER) DoGameOver(fElapsedTime);
		if(game.state == State::END_GAME) DoEndGame(fElapsedTime);
		if(game.state == State::CREDITS) DoCredits(fElapsedTime);
		return true;
	}

private: // State Functions

	void DoSplash(float fElapsedTime)
	{
		if(keyboard.GetKey(olc::Key::ESCAPE).bPressed)
		{
			game.state = State::NONE;
		}
		
		if(keyboard.GetKey(olc::Key::C).bPressed)
		{
			// switch to credits view
			game.state = State::CREDITS;
		}

		if(keyboard.GetKey(olc::Key::E).bPressed)
		{
			// set easy mode
			StartGame(60.0f);
		}

		if(keyboard.GetKey(olc::Key::H).bPressed)
		{
			// hard mode
			StartGame(30.0f);
		}

		// Draw splash screen sprite
		draw.Image(imgSplash, {0, 0});
	}
	
	void DoGame(float fElapsedTime)
	{
		game.time += fElapsedTime;

		if(game.time > game.gameOverTime)
		{
			game.state = State::GAMEOVER;
		}

		if(keyboard.GetKey(olc::Key::ESCAPE).bPressed)
		{
			game.state = State::SPLASH;
		}
		
		// force velocity to zero every frame, good for Top-Down RPG Style
		game.vel = { 0.0f, 0.0f };
		bool updateAnimation = false;
		if(keyboard.GetKey(olc::Key::UP).bHeld)
		{
			game.vel.y = -MOVE_SPEED * fElapsedTime;
			game.playerAnimationState = PlayerAnimationState::WALK_UP;
			updateAnimation = true;
		}
		
		if(keyboard.GetKey(olc::Key::DOWN).bHeld)
		{
			game.vel.y = MOVE_SPEED * fElapsedTime;
			game.playerAnimationState = PlayerAnimationState::WALK_DOWN;
			updateAnimation = true;
		}
		
		if(keyboard.GetKey(olc::Key::LEFT).bHeld)
		{
			game.vel.x = -MOVE_SPEED * fElapsedTime;
			game.playerAnimationState = PlayerAnimationState::WALK_LEFT;
			updateAnimation = true;
		}
		
		if(keyboard.GetKey(olc::Key::RIGHT).bHeld)
		{
			game.vel.x = MOVE_SPEED * fElapsedTime;
			game.playerAnimationState = PlayerAnimationState::WALK_RIGHT;
			updateAnimation = true;
		}
		
		if(updateAnimation)
		{
			animPlayer.ChangeState(game.animstate, game.playerAnimationState);
			animPlayer.UpdateState(game.animstate, fElapsedTime * 2.0f);
		}

		// COLLISIONS
		game.dpos = game.pos + game.vel;  // thanks javidx9
		
		// check for collisions on the x axis
		if(game.vel.x <= 0)
		{
			// left
			if(!IsWalkable({game.dpos.x, game.pos.y}, { 0.0f, 0.2f }) || !IsWalkable({game.dpos.x, game.pos.y}, { 0.0f, 0.8f }))
			{
				game.dpos.x = (int)game.dpos.x + 1;
				game.vel.x = 0;
			}
		}
		else
		{
			// right
			if(!IsWalkable({game.dpos.x, game.pos.y}, { 1.0f, 0.2f }) || !IsWalkable({game.dpos.x, game.pos.y}, { 1.0f, 0.8f }))
			{
				game.dpos.x = (int)game.dpos.x;
				game.vel.x = 0;
			}
		}

		// check for collisions on the y axis
		if(game.vel.y <= 0)
		{
			// up
			if(!IsWalkable({game.dpos.x, game.dpos.y}, { 0.2f, 0.0f }) || !IsWalkable({game.dpos.x, game.dpos.y}, { 0.8f, 0.0f }))
			{
				game.dpos.y = (int)game.dpos.y + 1;
				game.vel.y = 0;
			}
		}
		else
		{
			// down
			if(!IsWalkable({game.dpos.x, game.dpos.y}, { 0.2f, 1.0f }) || !IsWalkable({game.dpos.x, game.pos.y}, { 0.8f, 1.0f }))
			{
				game.dpos.y = (int)game.dpos.y;
				game.vel.y = 0;
			}
		}

		// update position after resolved collisions
		game.pos = game.dpos;

		// object collisions (not as strict as the tile collisions above)
		for(auto &obj : lObjects->getObjects())
		{
			// position of the object in world space
			olc::vf2d pos = {(float)obj.getPosition().x / TILE_SIZE, (float)obj.getPosition().y / TILE_SIZE};
			
			pos = pos - game.pos;

			// if magnitude is < 0.1f, trigger the things!
			if(pos.mag() < 0.1f)
			{
				if(obj.getType() == "town")
				{
					game.state = State::END_GAME;
				}
				
				if(obj.getType() == "teleport")
				{
					game.teleportPos.x = obj.get<int>("toX");
					game.teleportPos.y = obj.get<int>("toY");
					game.state = State::TELEPORT;

					break;
				}
			}

		}

		olc::Pixel tint;

		tint.r = uint8_t(255 - (game.time / (game.gameOverTime * 1.2f)) * 255);
		tint.g = tint.r;
		tint.b = tint.r;
	
		// DRAWING
		draw.Clear(olc::Colour::BLACK);
		DrawMap(fElapsedTime, tint);
		DrawHeadsUpDisplay(fElapsedTime);
		DrawCharacter(tint);
	}

// 	// teleport state
	void DoTeleport(float fElapsedTime)
	{
		static float fFadeDelay = 0.5;
		static float fFadeDelayTracker = 0.0f;
		static bool bFadeOut = true;
		olc::Pixel tint = olc::Colour::WHITE;
		
		float fProgress = game.time / (game.gameOverTime * 1.2f);

		// fade out
		if(bFadeOut)
		{
			// calculate tint
			tint.a = (uint8_t)(((1.0f - fProgress) * 255) - ((fFadeDelayTracker / fFadeDelay) * (1.0f - fProgress) * 255));
			tint.r = tint.g = tint.b = tint.a;

			// track the delay
			fFadeDelayTracker += fElapsedTime;
			if(fFadeDelayTracker > fFadeDelay)
			{
				// set player position to the teleport position
				game.pos.x = game.teleportPos.x;
				game.pos.y = game.teleportPos.y;
				
				// set variables to trigger fade in
				fFadeDelayTracker = 0.0f;
				bFadeOut = false;
			}
		}
		
		// fade in
		if(!bFadeOut)
		{
			// calculate tint
			tint.a = (uint8_t)((fFadeDelayTracker / fFadeDelay) * (1.0f - fProgress) * 255);
			tint.r = tint.g = tint.b = tint.a;

			// track the delay
			fFadeDelayTracker += fElapsedTime;
			if(fFadeDelayTracker > fFadeDelay)
			{
				// switch to GAME state
				game.state = State::GAME;
				
				// reset static variables to default state
				fFadeDelayTracker = 0.0f;
				bFadeOut = true;
			}
		}
		
		DrawMap(fElapsedTime, tint);
		DrawHeadsUpDisplay(fElapsedTime);
		DrawCharacter(tint);
	}
	
	// game over state
	void DoGameOver(float fElapsedTime)
	{
		if(keyboard.GetKey(olc::Key::ESCAPE).bPressed || keyboard.GetKey(olc::Key::SPACE).bPressed)
		{
			game.state = State::SPLASH;
		}
		
		draw.Clear(olc::Colour::BLACK);
		draw.String(
			olc::vi2d{(ScreenSize().x / 2) - (8 * 4 * 2), (ScreenSize().y / 2) - 12},
			"GAME OVER",
			olc::Colour::WHITE,
			{2.0f, 2.0f}
		);
		
		draw.String(
			olc::vi2d{(ScreenSize().x / 2) - (8 * 15 * 1), (ScreenSize().y / 2) + 12},
			"Press ESC or SPACE to Try Again",
			olc::Colour::WHITE
		);
	}
	
	// end game state
	void DoEndGame(float fElapsedTime)
	{
		static float fDelayTracker = 0.0f;

		fDelayTracker += fElapsedTime;
		if(fDelayTracker > 3.0f)
		{
			fDelayTracker = 0.0f;
			game.state = State::CREDITS;
		}
		draw.Clear(olc::Colour::BLACK);
		draw.String(olc::vi2d{
				(ScreenSize().x / 2) - (8 * 8 * 2),
				(ScreenSize().y / 2) - 32
			},
			"Congratulations!",
			olc::Colour::WHITE,
			{ 2.0f, 2.0f }
		);
		
		draw.String(olc::vi2d{
				(ScreenSize().x / 2) - (8 * 8 * 2),
				(ScreenSize().y / 2) - 12
			},
			"You Made IT!!!!!",
			olc::Colour::WHITE,
			{ 2.0f, 2.0f }
		);
	}
	
	// credits state
	void DoCredits(float fElapsedTime)
	{
		static float fScrollTracker = 0.0f;

		if(keyboard.GetKey(olc::Key::ESCAPE).bPressed)
		{
			game.state = State::SPLASH;
			fScrollTracker = 0.0f;
		}

		fScrollTracker += fElapsedTime * 40.0f;
		
		if(fScrollTracker > (imgCredits.Size().y + ScreenSize().y))
			fScrollTracker = 0.0f;
		
		draw.Clear(olc::Colour::BLACK);
		draw.Image(imgCredits, {0, ScreenSize().y + -fScrollTracker });
	}


private:
	void DrawCharacter(olc::Pixel tint = olc::Colour::WHITE)
	{
		draw.Image(
			imgShadow,
			{
				(ScreenSize().x / 2) - TILE_SIZE + 0.0f,
				(ScreenSize().y / 2) - (TILE_SIZE * 1.6f) + 0.0f
			},
			{ 1.0f, 1.0f },
			tint
		);
		
		draw.Image(
			animPlayer.GetFrame(game.animstate),
			{
				(ScreenSize().x / 2) - TILE_SIZE + 0.0f,
				(ScreenSize().y / 2) - (TILE_SIZE * 1.8f) + 0.0f
			},
			{ 1.0f, 1.0f },
			tint
		);
	}

	// helper function draws the map at the current player position
	void DrawMap(float fElapsedTime, olc::Pixel tint = olc::Colour::WHITE)
	{
		olc::vf2d vCameraPos = game.pos;
		olc::vf2d vVisibleTiles = ScreenSize() / TILE_SIZE;
		olc::vf2d vCameraOffset = vCameraPos - vVisibleTiles / 2.0f;

		// Get offsets for smooth movement
		olc::vf2d vTileOffset = {
			(vCameraOffset.x - (int)vCameraOffset.x) * TILE_SIZE,
			(vCameraOffset.y - (int)vCameraOffset.y) * TILE_SIZE
		};

		for(int y = 0; y < vVisibleTiles.y + 2; y++)
		{
			olc::vf2d temp;

			temp.y = ((y - 0.5f) * TILE_SIZE) - vTileOffset.y;

			for(int x = 0; x < vVisibleTiles.x + 2; x++)
			{
				temp.x = ((x - 0.5f) * TILE_SIZE) - vTileOffset.x;

				for(auto &layer : tMap->getLayers())
				{
					tile = layer.getTileData(x + vCameraOffset.x, y + vCameraOffset.y);
					
					if(tile != nullptr)
					{
						draw.Image(imgTileset.region(TilePosition(tile), { TILE_SIZE, TILE_SIZE }), temp, { 1.0f, 1.0f }, tint);
					}
				}
			}
		}
	}
	
	void DrawHeadsUpDisplay(float fElapsedTime)
	{
		float fProgress = game.time / game.gameOverTime;
		
		draw.Image(imgHeadsUpDisplay, {210, 210});
		draw.FilledRect({211, 221}, {99 * fProgress, 3}, olc::Colour::VERY_DARK_GREY);
	}
	
	// helper function to reset all game variables and kick off the GAME state
	void StartGame(float time)
	{
		// set game over time tracker
		game.gameOverTime = time;
		
		// set game time
		game.time = 0.0f;

		// set starting point
		game.pos.x = (float)lObjects->firstObj("player")->getPosition().x / TILE_SIZE;
		game.pos.y = (float)lObjects->firstObj("player")->getPosition().y / TILE_SIZE;

		// change to main game state
		game.state = State::GAME;
	}

	// helper function for tile collisions
	bool IsWalkable(const olc::vf2d &pos, const olc::vf2d &offset = { 0.0f, 0.0f })
	{
		bool ret = true;

		// top most layer takes precedence of the layers beneath it
		for(auto &layer : tMap->getLayers())
		{
			tson::Tile *tile = layer.getTileData(pos.x + offset.x, pos.y + offset.y);
			if(tile != nullptr)
			{
				if(tile->get<bool>("Walkable"))
					ret = true;
				else
					ret = false;
			}
		}
		
		return ret;
	}

	// helper function to get a tile's position in the tileset, for drawing
	olc::vi2d TilePosition(const tson::Tile *t)
	{
		static int tilesetWidth = 0;

		if(tilesetWidth == 0)
			tilesetWidth = tTileset->getImageSize().x / (TILE_SIZE + 2);
		
		int id = t->getId()-1;

		return { ((id % tilesetWidth) * (TILE_SIZE + 2))+1, ((id / tilesetWidth) * (TILE_SIZE + 2))+1 };
	}

	enum class PlayerAnimationState: uint8_t
	{
		WALK_UP,
		WALK_LEFT,
		WALK_DOWN,
		WALK_RIGHT,
	};

	olc::utils::Animate2D::Animation<PlayerAnimationState> animPlayer;

	struct Game {
		int state{0};
		olc::vf2d pos;
		olc::vf2d dpos;
		olc::vf2d vel;
		olc::vi2d teleportPos;
		PlayerAnimationState playerAnimationState{PlayerAnimationState::WALK_DOWN};
		float time;
		float gameOverTime;
		
		// magick?
		olc::utils::Animate2D::AnimationState animstate;
	};

private:
	Game game;
	
	tson::Tileson tParser;
	std::unique_ptr<tson::Map> tMap;
	tson::Tileset* tTileset;
	tson::Tile* tile;
	tson::Layer* lObjects;

	olc::vi2d tileSize;

	olc::Image imgCharacter;
	olc::Image imgTileset;
	olc::Image imgHeadsUpDisplay;
	olc::Image imgShadow;
	olc::Image imgSplash;
	olc::Image imgCredits;
};

int main()
{
	GetHome_Game game;
	
	olc::PGEConfig cfg;
	cfg.vScreenSize = {320, 240};
	cfg.vPixelSize = {2, 2};
	cfg.bVSync = false;
	
	if(game.Construct(cfg))
		game.Start();

	return 0;
}
