#pragma once


#include "Collider.h"
#include "Moveable.h"
#include "Timer.h"
class MoveablePlayer :public Moveable, public Callable
{
public:
	MoveablePlayer(Tmpl8::vec2* pos, Collider* col, const Collider* tileMapCol, float speed = 70.0f, float dashSpeed = 500);
	MoveablePlayer();
	void Init(Tmpl8::vec2* pos, Collider* col, const Collider* tileMapCol, float speed = 70.0f, float dashSpeed = 500);
	virtual ~MoveablePlayer();

	void Update(float deltaTime) override;
	void MoveTileOrPlayer(Tmpl8::vec2& tilemapPos, Collider& c, const Tmpl8::vec2& playerPos);
	bool CheckPositionForCollisions(const Tmpl8::vec2& playerPos, const Collider& playerCol);
	void SetDashPos(Tmpl8::vec2& nextPos);
	void ResetTriggers();
	void MovePlayer();
	void ClampTheMovementVector(const Collider& c, const Tmpl8::vec2 newVec, Tmpl8::vec2& originalVec, bool& changed);
	bool CheckVecForOneDir(Tmpl8::vec2& nextPos);
	void ClampTheMovementVector(const Collider& c, const Tmpl8::vec2 newVec, Tmpl8::vec2& originalVec);

	void setUp(bool val = false);
	void setDown(bool val = false);
	void setRight(bool val = false);
	void setLeft(bool val = false);
	void setDash(bool val = false);

	void startDash();

	void Dashing();

	float GetDashLinearTime() const;
	bool CanRotate()const;
	bool IsDashing() const;
	bool ChangedPos() const;
private:
	void InitTimers();
	bool hasChangedPos = false;
	bool tileDiagonalMoved = false;
	bool canMove = false;
	bool canRotate = false;
	//wasd
	bool up = false;
	bool down = false;
	bool right = false;
	bool left = false;

	const Collider* tileMapCol;
	Tmpl8::vec2 playerMovement;

	//dash
	Timer dashTimer;
	Timer cooldownTimer;
	Tmpl8::vec2 dashDir = { 0 };
	bool dashing = false;
	bool startedDashing = false;
	int dashes = 0;
	float initSpeed;
	float dashSpeed;
	float timePassed = 0.0f;
	float linearT = 0.0f;
	//consts
	const float DASH_DURATION = 0.45f;
	const float COOLDOWN_DURATION = 0.6f;
	const float EDGE_DISTANCE = 5.0f;
	// Inherited via Callable
	virtual void Call() override;

};

