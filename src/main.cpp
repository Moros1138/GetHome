#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_GRAPHICS2D
#include "olcPGEX_Graphics2D.h"

#define OLC_PGEX_ANIMSPR
#include "olcPGEX_AnimatedSprite.h"

#include "tileson.hpp"

#include <cstdio>

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

struct Game {
	int state;
	olc::AnimatedSprite sprite;
	olc::vf2d pos;
	olc::vf2d dpos;
	olc::vf2d vel;
	olc::vi2d teleportPos;
	float time;
	float gameOverTime;
};


class olc_BeatTheBoredom : public olc::PixelGameEngine
{
public:
	olc_BeatTheBoredom()
	{
		sAppName = "BeatTheBoredom";
	}

public:
	bool OnUserCreate() override
	{
		game.state = SPLASH;

		pack = new olc::ResourcePack();
		pack->LoadPack("assets/resource.pak", "moros rocks");
		
		if(!pack->Loaded())
		{
			std::cout << "Oh fuck!" << std::endl;
			return false;
		}

		tilesetSprite = new olc::Sprite("assets/olcBTB_tileset1.png", pack);
		tilesetDecal = new olc::Decal(tilesetSprite);

		hudSprite = new olc::Sprite(110, 16);
		hudDecal = new olc::Decal(hudSprite);

		characterSprite = new olc::Sprite(32, 32);
		characterDecal = new olc::Decal(characterSprite);

		shadowSprite = new olc::Sprite(32, 32);

		SetDrawTarget(shadowSprite);
		Clear(olc::BLANK);
		FillCircle(characterSprite->width / 2, characterSprite->height - 6, 6, olc::Pixel(0, 0, 20, 128));
		SetDrawTarget(nullptr);

		shadowDecal = new olc::Decal(shadowSprite);

		splashSprite = new olc::Sprite("assets/olcBTB_splash.png", pack);
		creditsSprite = new olc::Sprite("assets/olcBTB_credits.png", pack);
		
		LoadCharacterSprite();

		olc::ResourceBuffer rb = pack->GetFileBuffer("assets/outdoors.json");
		
		map = parser.parse(rb.vMemory.data(), rb.vMemory.size());
		tileset = map.getTileset("olcBTB_tileset1");

		objects = map.getLayer("objects");
		
		mapRenderSize = { ScreenWidth() / TILE_SIZE, ScreenHeight() / TILE_SIZE };

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
		if(GetKey(olc::ESCAPE).bPressed)
		{
			game.state = State::NONE;
		}
		
		if(GetKey(olc::C).bPressed)
		{
			// switch to credits view
			game.state = State::CREDITS;
		}

		if(GetKey(olc::E).bPressed)
		{
			// set easy mode
			StartGame(60.0f);
		}

		if(GetKey(olc::H).bPressed)
		{
			// hard mode
			StartGame(30.0f);
		}

		// Draw splash screen sprite
		DrawSprite(0, 0, splashSprite, 1);
	}
	
	void DoGame(float fElapsedTime)
	{
		game.time += fElapsedTime;

		if(game.time > game.gameOverTime)
		{
			game.state = State::GAMEOVER;
		}

		if(GetKey(olc::ESCAPE).bPressed)
		{
			game.state = State::SPLASH;
		}
		
		// force velocity to zero every frame, good for Top-Down RPG Style
		game.vel = { 0.0f, 0.0f };

		// PLAYER INPUT
		if(GetKey(olc::UP).bHeld)
		{
			game.vel.y = -MOVE_SPEED * fElapsedTime;
			game.sprite.SetState("up");
		}

		if(GetKey(olc::DOWN).bHeld)
		{
			game.vel.y = MOVE_SPEED * fElapsedTime;
			game.sprite.SetState("down");
		}
		
		if(GetKey(olc::LEFT).bHeld)
		{
			game.vel.x = -MOVE_SPEED * fElapsedTime;
			game.sprite.SetState("left");
		}
		
		if(GetKey(olc::RIGHT).bHeld)
		{
			game.vel.x = MOVE_SPEED * fElapsedTime;
			game.sprite.SetState("right");
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
		for(auto &obj : objects->getObjects())
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
		DrawMap(fElapsedTime, tint);
		DrawHUD(fElapsedTime);
		DrawCharacter(fElapsedTime, tint);
	}

	// teleport state
	void DoTeleport(float fElapsedTime)
	{
		static float fFadeDelay = 0.5;
		static float fFadeDelayTracker = 0.0f;
		static bool bFadeOut = true;
		olc::Pixel tint = olc::WHITE;
		
		float fProgress = game.time / (game.gameOverTime * 1.2f);

		// fade out
		if(bFadeOut)
		{
			// calculate tint
			tint.a = (uint8_t)(((1.0f - fProgress) * 255) - ((fFadeDelayTracker / fFadeDelay) * (1.0f - fProgress) * 255));
			tint.r = tint.a;
			tint.g = tint.a;
			tint.b = tint.a;

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
			tint.r = tint.a;
			tint.g = tint.a;
			tint.b = tint.a;

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
		DrawHUD(fElapsedTime);
		DrawCharacter(fElapsedTime, tint);
	}
	
	// game over state
	void DoGameOver(float fElapsedTime)
	{
		if(GetKey(olc::ESCAPE).bPressed || GetKey(olc::SPACE).bPressed)
		{
			game.state = State::SPLASH;
		}
		
		Clear(olc::BLACK);
		DrawString((ScreenWidth() / 2) - (8 * 4 * 2), (ScreenHeight() / 2) - 12, "GAME OVER", olc::WHITE, 2);
		DrawString((ScreenWidth() / 2) - (8 * 15 * 1), (ScreenHeight() / 2) + 12, "Press ESC or SPACE to Try Again", olc::WHITE, 1);
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
		
		Clear(olc::BLACK);
		DrawString((ScreenWidth() / 2) - (8 * 8 * 2), (ScreenHeight() / 2) - 32, "Congratulations!", olc::WHITE, 2);
		DrawString((ScreenWidth() / 2) - (8 * 8 * 2), (ScreenHeight() / 2) - 12, "You Made IT!!!!!", olc::WHITE, 2);
	}
	
	// credits state
	void DoCredits(float fElapsedTime)
	{
		static float fScrollTracker = 0.0f;

		if(GetKey(olc::ESCAPE).bPressed)
		{
			game.state = State::SPLASH;
			fScrollTracker = 0.0f;
		}

		fScrollTracker += fElapsedTime * 40.0f;
		
		if(fScrollTracker > (creditsSprite->height + ScreenHeight()))
			fScrollTracker = 0.0f;

		Clear(olc::BLACK);
		DrawSprite(0, ScreenHeight() + -fScrollTracker , creditsSprite, 1);
	}


private:
	void DrawCharacter(float fElapsedTime, olc::Pixel tint = olc::WHITE)
	{
		SetDrawTarget(characterSprite);
		game.sprite.Draw(fElapsedTime, {0, 0});
		SetDrawTarget(nullptr);

		DrawDecal({(ScreenWidth() / 2) - TILE_SIZE + 0.0f, (ScreenHeight() / 2) - (TILE_SIZE * 1.6f) + 0.0f}, shadowDecal, {1.0f, 1.0f}, tint);

		characterDecal->Update();
		DrawDecal({(ScreenWidth() / 2) - TILE_SIZE + 0.0f, (ScreenHeight() / 2) - (TILE_SIZE * 1.6f) + 0.0f}, characterDecal, {1.0f, 1.0f}, tint);

	}

	// helper function draws the map at the current player position
	void DrawMap(float fElapsedTime, olc::Pixel tint = olc::WHITE)
	{
		Clear(olc::BLACK);
		
		olc::vf2d offset {(float)fmod(game.pos.x * TILE_SIZE, TILE_SIZE) + TILE_SIZE / 2, (float)fmod(game.pos.y * TILE_SIZE, TILE_SIZE) + (ScreenHeight() % TILE_SIZE) / 2 };

		for(int y = -2; y < mapRenderSize.y + 2; y++)
		{
			for(int x = -2; x < mapRenderSize.x + 2; x++)
			{
				for(auto &layer : map.getLayers())
				{
					tile = layer.getTileData(x + game.pos.x - (mapRenderSize.x / 2), y + game.pos.y - (mapRenderSize.y / 2));
					if(tile != nullptr)
					{
						olc::vi2d temp = {x * TILE_SIZE - (int)offset.x, y * TILE_SIZE - (int)offset.y};
						DrawPartialDecal(temp, tilesetDecal, TilePosition(tile), { TILE_SIZE, TILE_SIZE }, { 1.0f, 1.0f }, tint);
					}
				}
			}
		}
	}
	
	void DrawHUD(float fElapsedTime)
	{
		float fProgress = game.time / game.gameOverTime;
		
		SetDrawTarget(hudSprite);
		Clear(olc::BLANK);
		DrawString(1, 1, "Time", olc::BLACK, 1);
		DrawString(0, 0, "Time", olc::WHITE, 1);
		
		FillRect(2, 12, 100, 4, olc::BLACK);
		FillRect(0, 10, 100, 4, olc::YELLOW);
		FillRect(0, 10, 100 * fProgress, 4, olc::VERY_DARK_GREY);
		DrawRect(0, 10, 100, 4, olc::WHITE);
		SetDrawTarget(nullptr);
		
		hudDecal->Update();
		DrawDecal({ 210, 210 }, hudDecal);
	}

	// loads and sets the animated character sprite states
	void LoadCharacterSprite()
	{
		game.sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
		game.sprite.spriteSheet = new olc::Sprite("assets/olcBTB_character.png", pack); // define image to use for the spritesheet
		
		game.sprite.SetSpriteSize({32, 32}); // define size of each sprite with an olc::vi2d
		game.sprite.SetSpriteScale(1.0f); // define scale of sprite; 1.0f is original size. Must be above 0 and defaults to 1.0f

		// define "up" sprite
		game.sprite.AddState("up", std::vector<olc::vi2d>{
			{0, 0}, {32, 0}, {64, 0}, {96, 0}, {128, 0}, {160, 0}, {192, 0}, {224, 0}, {256, 0}
		});

		// define "left" sprite
		game.sprite.AddState("left", std::vector<olc::vi2d>{
			{0, 32}, {32, 32}, {64, 32}, {96, 32}, {128, 32}, {160, 32}, {192, 32}, {224, 32}, {256, 32},
		});

		// define "down" sprite
		game.sprite.AddState("down", std::vector<olc::vi2d>{
			{0, 64}, {32, 64}, {64, 64}, {96, 64}, {128, 64}, {160, 64}, {192, 64}, {224, 64}, {256, 64}
		});

		// define "right" sprite
		game.sprite.AddState("right", std::vector<olc::vi2d>{
			{0, 96}, {32, 96}, {64, 96}, {96, 96}, {128, 96}, {160, 96}, {192, 96}, {224, 96}, {256, 96}
		});

		// set initial state
		game.sprite.SetState("down");
	}
	
	// helper function to reset all game variables and kick off the GAME state
	void StartGame(float time)
	{
		// set game over time tracker
		game.gameOverTime = time;
		
		// set game time
		game.time = 0.0f;

		// set starting point
		game.pos.x = (float)objects->firstObj("player")->getPosition().x / TILE_SIZE;
		game.pos.y = (float)objects->firstObj("player")->getPosition().y / TILE_SIZE;

		// change to main game state
		game.state = State::GAME;
	}

	// helper function for tile collisions
	bool IsWalkable(const olc::vf2d &pos, const olc::vf2d &offset = { 0.0f, 0.0f })
	{
		bool ret = true;

		// top most layer takes precedence of the layers beneath it
		for(auto &layer : map.getLayers())
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
			tilesetWidth = tileset->getImageSize().x / TILE_SIZE;
		
		int id = t->getId()-1;

		return { (id % tilesetWidth) * TILE_SIZE, (id / tilesetWidth) * TILE_SIZE };
	}

private:
	Game game;
	
	olc::ResourcePack *pack;

	tson::Tileson parser;
	tson::Map map;
	tson::Tileset *tileset;
	tson::Tile *tile;
	tson::Layer *objects;

	olc::vi2d tileSize;

	olc::Sprite *tilesetSprite;
	olc::Decal *tilesetDecal;

	olc::Sprite *hudSprite;
	olc::Decal *hudDecal;

	olc::Sprite *characterSprite;
	olc::Decal *characterDecal;
	
	olc::Sprite *shadowSprite;
	olc::Decal *shadowDecal;

	olc::Sprite *splashSprite;
	olc::Sprite *creditsSprite;

	olc::vi2d mapRenderSize;
};

int main()
{
	olc_BeatTheBoredom game;
	if(game.Construct(320, 240, 2, 2, false, false))
		game.Start();

	return 0;
}
