#pragma once
#include "Subject.h"
#include "Being.h"
#include "ProjectileSpawner.h"
#include "RotationVar.h"
#include "MoveablePlayer.h"
#include "Tilemap.h"
#include "ClippedSprite.h"
#include "HealthBar.h"


class Player :public Being, public Followable, public Observer, public Subject
{
public:
	Player();
	void Init(const Collider& tilemapCollider, const Tmpl8::vec2& pos);
	~Player() = default;
	void Render(Tmpl8::Surface* screen);
	void Update(float deltaTime);
	void Shoot(bool fire);
	void Rotate(int x, int y);
	//callable
	void Call();
	//being  override
	void TakeDamage(int) override;
	void Die() override;
	//followable override
	const Tmpl8::vec2 GetOffset() override;
	void ResetOffset() override;

	//getters
	MoveablePlayer* GetMoveable();
	ProjectileSpawner* GetSpawner();
	const Tmpl8::vec2 GetDir() const;
	const Tmpl8::vec2 GetPos() const;
	//just returns the x of the max part of the collider
	const float GetHalfCollider() const;
private:
	RotationVar rVar;
	Tmpl8::vec2 dirToFace;
	Tmpl8::vec2 startingPos;
	//src/assets
	const std::filesystem::path spriteProjectilePath = "src/assets/OriginalAssets/phaser.tga";
	const std::filesystem::path spriteExplosionPath = "src/assets/OriginalAssets/smoke.tga";
	//consts
	const float TIME_TO_HIT = 0.5f;
	const Tmpl8::vec2 COL_MIN = Tmpl8::vec2(-21.0f);
	const Tmpl8::vec2 COL_MAX = Tmpl8::vec2(21.0f);
	//components
	const Collider* tilemapCollider;
	MoveablePlayer playerMover;
	Collider playerCollider;
	ProjectileSpawner spawner;
	Timer cooldownForDamage;
	HealthBar hpBar;
	// Inherited via Observer
	void onNotify(int points, EventType event) override;

};
inline const float Player::GetHalfCollider()const {
	return COL_MAX.x;
}
inline const Tmpl8::vec2 Player::GetOffset() {
	return   -pos + lastPos;
}

inline void Player::ResetOffset()
{
	lastPos = pos;

}





