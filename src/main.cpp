#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

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
		pack = new olc::ResourcePack();
		pack->LoadPack("assets/resource.pak", "moros rocks");
		
		if(!pack->Loaded())
		{
			std::cout << "Oh fuck!" << std::endl;
			return false;
		}

		sprTileset = new olc::Sprite("assets/olcBTB_tileset1.png", pack);
		decTileset = new olc::Decal(sprTileset);

		sprHUD = new olc::Sprite(110, 16);

		SetDrawTarget(sprHUD);
		Clear(olc::BLANK);
		DrawString(1, 1, "Time", olc::BLACK, 1);
		DrawString(0, 0, "Time", olc::WHITE, 1);
		
		FillRect(2, 12, 100, 4, olc::BLACK);
		FillRect(0, 10, 100, 4, olc::YELLOW);
		DrawRect(0, 10, 100, 4, olc::WHITE);
		SetDrawTarget(nullptr);

		decHUD = new olc::Decal(sprHUD);

		sprOnePixel = new olc::Sprite(1, 1);

		SetDrawTarget(sprOnePixel);
		Clear(olc::WHITE);
		SetDrawTarget(nullptr);

		decOnePixel = new olc::Decal(sprOnePixel);

		sprShadow = new olc::Sprite(32, 32);

		SetDrawTarget(sprShadow);
		Clear(olc::BLANK);
		FillCircle(sprShadow->width / 2, sprShadow->height - 6, 6, olc::Pixel(0, 0, 20, 128));
		SetDrawTarget(nullptr);

		decShadow = new olc::Decal(sprShadow);

		sprSplash = new olc::Sprite("assets/olcBTB_splash.png", pack);
		decSplash = new olc::Decal(sprSplash);

		sprCredits = new olc::Sprite("assets/olcBTB_credits.png", pack);
		decCredits = new olc::Decal(sprCredits);

		LoadCharacterSprite();

		olc::ResourceBuffer rb = pack->GetFileBuffer("assets/outdoors.json");
		
		tMap = tParser.parse(rb.vMemory.data(), rb.vMemory.size());
		tTileset = tMap.getTileset("olcBTB_tileset1");

		lObjects = tMap.getLayer("objects");
		
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
		DrawDecal({0, 0}, decSplash);
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
		tint.g = tint.b = tint.r;

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
		
		DrawStringDecal({(float)(ScreenWidth() / 2) - (8 * 4 * 2), (float)(ScreenHeight() / 2) - 12}, "GAME OVER", olc::WHITE, {2.0f, 2.0f});
		DrawStringDecal({(float)(ScreenWidth() / 2) - (8 * 15 * 1), (float)(ScreenHeight() / 2) + 12}, "Press ESC or SPACE to Try Again", olc::WHITE);
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
		
		DrawStringDecal({(float)(ScreenWidth() / 2) - (8 * 8 * 2), (float)(ScreenHeight() / 2) - 32}, "Congratulations!", olc::WHITE, {2.0f, 2.0f});
		DrawStringDecal({(float)(ScreenWidth() / 2) - (8 * 8 * 2), (float)(ScreenHeight() / 2) - 12}, "You Made IT!!!!!", olc::WHITE, {2.0f, 2.0f});
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
		
		if(fScrollTracker > (sprCredits->height + ScreenHeight()))
			fScrollTracker = 0.0f;

		DrawDecal({0, ScreenHeight() + -fScrollTracker }, decCredits);
	}


private:
	void DrawCharacter(float fElapsedTime, olc::Pixel tint = olc::WHITE)
	{
		DrawDecal(
			{
				(ScreenWidth() / 2) - TILE_SIZE + 0.0f,
				(ScreenHeight() / 2) - (TILE_SIZE * 1.6f) + 0.0f
			},
			decShadow,
			{1.0f, 1.0f},
			tint
		);
		
		game.sprite.Draw(
			fElapsedTime,
			{
				(ScreenWidth() / 2) - TILE_SIZE + 0.0f,
				(ScreenHeight() / 2) - (TILE_SIZE * 1.8f) + 0.0f
			},
			olc::Sprite::Flip::NONE,
			tint
		);
	}

	// helper function draws the map at the current player position
	void DrawMap(float fElapsedTime, olc::Pixel tint = olc::WHITE)
	{
		olc::vf2d vCameraPos = game.pos;
		olc::vf2d vVisibleTiles = {
			(float)ScreenWidth() / TILE_SIZE,
			(float)ScreenHeight() / TILE_SIZE
		};

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

				for(auto &layer : tMap.getLayers())
				{
					tile = layer.getTileData(x + vCameraOffset.x, y + vCameraOffset.y);
					
					if(tile != nullptr)
					{
						DrawPartialDecal(
							temp,
							decTileset,
							TilePosition(tile),
							{
								TILE_SIZE,
								TILE_SIZE
							},
							{
								1.0f,
								1.0f
							},
							tint
						);
					}
				}
			}
		}
	}
	
	void DrawHUD(float fElapsedTime)
	{
		float fProgress = game.time / game.gameOverTime;
		
		DrawDecal({210, 210}, decHUD);
		DrawDecal({211, 221}, decOnePixel, {99 * fProgress, 3}, olc::VERY_DARK_GREY);
	}

	// loads and sets the animated character sprite states
	void LoadCharacterSprite()
	{
		game.sprite.mode = olc::AnimatedSprite::SPRITE_MODE::SINGLE; // set sprite to use a single spritesheet
		game.sprite.type = olc::AnimatedSprite::SPRITE_TYPE::DECAL;

		game.sprite.spriteSheet = new olc::Renderable("assets/olcBTB_character.png", pack); // define image to use for the spritesheet
		
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
		for(auto &layer : tMap.getLayers())
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

private:
	Game game;
	
	olc::ResourcePack *pack;

	tson::Tileson tParser;
	tson::Map tMap;
	tson::Tileset *tTileset;
	tson::Tile *tile;
	tson::Layer *lObjects;

	olc::vi2d tileSize;

	olc::Sprite *sprTileset;
	olc::Decal *decTileset;

	olc::Sprite *sprHUD;
	olc::Decal *decHUD;

	olc::Sprite *sprOnePixel;
	olc::Decal *decOnePixel;

	olc::Sprite *sprShadow;
	olc::Decal *decShadow;

	olc::Sprite *sprSplash;
	olc::Decal *decSplash;
	
	olc::Sprite *sprCredits;
	olc::Decal *decCredits;
};

int main()
{
	olc_BeatTheBoredom game;
	
	if(game.Construct(320, 240, 2, 2, false, false))
		game.Start();

	return 0;
}
