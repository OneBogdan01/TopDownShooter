#include "MoveablePlayer.h"
#include "MathFunctions.h"
#include "game.h"

#include <iostream>

MoveablePlayer::MoveablePlayer(Tmpl8::vec2* pos, Collider* col, Collider* tileMapCol, float speed, float DashSpeed) :
	Moveable(pos, col, speed),
	dashTimer(Timer()),
	cooldownTimer(Timer()),
	tileMapCol(tileMapCol)

{
	initSpeed = speed;
	dashSpeed = DashSpeed;
	//timer init
	cooldownTimer.Init(this, COOLDOWN_DURATION);
	cooldownTimer.isUpdateable = false;
	dashTimer.Init(this, DASH_DURATION);
	dashTimer.isUpdateable = false;
}

MoveablePlayer::~MoveablePlayer()
{
}
void MoveablePlayer::copyInput(MoveablePlayer& p) {
	p.setDash(startedDashing);
	p.setDown(down);
	p.setLeft(left);
	p.setRight(right);
	p.setUp(up);
}
void MoveablePlayer::setUp(bool val)
{
	up = val;
}

void MoveablePlayer::setDown(bool val)
{
	down = val;
}

void MoveablePlayer::setRight(bool val)
{
	right = val;
}

void MoveablePlayer::setLeft(bool val)
{
	left = val;
}

void MoveablePlayer::setDash(bool val)
{
	startedDashing = val;
}



void MoveablePlayer::startDash()
{
	if (!dashTimer.isUpdateable && !dashing) {

		dashes = 0;

		dashTimer.isUpdateable = true;

		Dashing();
	}
}

void MoveablePlayer::Dashing()
{
	dashing = true;
	timePassed = 0;
	speed = dashSpeed;
}

float MoveablePlayer::GetDashLinearTime() const
{
	return linearT;
}

bool MoveablePlayer::CanRotate() const
{
	return canRotate;
}

bool MoveablePlayer::IsDashing() const
{
	return dashTimer.isUpdateable;
}

bool MoveablePlayer::ChangedPos() const
{
	return hasChangedPos;
}

void MoveablePlayer::Call()
{


	if (dashTimer.isUpdateable) {
		dashTimer.ResetVar();
		dashTimer.isUpdateable = false;
		speed = initSpeed;
	}
	else if (cooldownTimer.isUpdateable) {
		cooldownTimer.ResetVar();
		cooldownTimer.isUpdateable = false;
		dashing = false;
	}

}


void MoveablePlayer::Update(float deltaTime)
{
	ResetTriggers();
	dashTimer.Update(deltaTime);
	cooldownTimer.Update(deltaTime);

	Tmpl8::vec2 nextPos = { 0 };
	//no movement when dashing

	if (up) {
		nextPos.y--;
	}
	if (down) {
		nextPos.y++;
	}
	if (right) {
		nextPos.x++;
	}

	if (left) {
		nextPos.x--;
	}
	//if the player goes on a diagonal the dynamic_array will not have a maginitude of 1
	if (nextPos.length() > 0)
		nextPos.normalize();

	//dashing
	if ((nextPos.x != 0 || nextPos.y != 0)) {
		canRotate = true;
		if (startedDashing) {
			dashDir = nextPos;
			startDash();
		}
	}
	else
		canRotate = false;

	if (dashTimer.isUpdateable == false && dashing && !startedDashing) {
		//start the cooldown then the dash is finished
		cooldownTimer.isUpdateable = true;
	}

	if (dashing && timePassed + deltaTime < DASH_DURATION) {

		timePassed += deltaTime;
		SetDashPos(nextPos);;//this value is from the function graph
	}

	//tile check
	Tmpl8::vec2 playerPos = *pos;
	Collider playerCol = *col;


	float playerPosOnX = nextPos.x * speed * deltaTime;
	float  playerPosOnY = nextPos.y * speed * deltaTime;

	playerPos += nextPos * speed * deltaTime;
	//tilemap
	Tmpl8::vec2 tilemapPos = *tileMapCol->pos;
	Collider c = *tileMapCol;
	tilemapPos += nextPos * (-speed) * deltaTime;
	if (CheckPositionForCollisions(playerPos, playerCol)) {
		MoveTileOrPlayer(tilemapPos, c, playerPos);
	}
	//moving diagonally and hitting an obstacle
	else if (nextPos.x != 0 && nextPos.y != 0) {
		if (CheckPositionForCollisions((*pos) + Tmpl8::vec2(0, playerPosOnY), playerCol)) {
			MoveTileOrPlayer((*c.pos) - Tmpl8::vec2(0, playerPosOnY), c, (*pos) + Tmpl8::vec2(0, playerPosOnY));
		}
		else if (CheckPositionForCollisions((*pos) + Tmpl8::vec2(playerPosOnX, 0), playerCol)) {
			MoveTileOrPlayer((*c.pos) - Tmpl8::vec2(playerPosOnX, 0), c, (*pos) + Tmpl8::vec2(playerPosOnX, 0));
		}
	}
}

void MoveablePlayer::MoveTileOrPlayer(Tmpl8::vec2& tilemapPos, Collider& c, const Tmpl8::vec2& playerPos)
{
	//move tilemap if it does not hit the bounds
	if (Collider::TileMapInGameScreen(tilemapPos, c)) {
		*tileMapCol->pos = tilemapPos;
		hasChangedPos = true;

	}
	else
	{
		//this would also move the player so we need to limit the speed
		tileDiagonalMoved = false;
		ClampTheMovementVector(c, tilemapPos, *tileMapCol->pos, tileDiagonalMoved);

	}

	canMove = true;
	playerMovement = playerPos;
}



bool MoveablePlayer::CheckPositionForCollisions(const Tmpl8::vec2& playerPos, Collider& playerCol, Collider& obs)
{
	return Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.min.x, playerPos.y + playerCol.min.y, obs) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.max.x, playerPos.y + playerCol.min.y, obs) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.min.x, playerPos.y + playerCol.max.y, obs) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.max.x, playerPos.y + playerCol.max.y, obs);
}
bool MoveablePlayer::CheckPositionForCollisions(const Tmpl8::vec2& playerPos, Collider& playerCol)
{
	return Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.min.x, playerPos.y + playerCol.min.y) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.max.x, playerPos.y + playerCol.min.y) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.min.x, playerPos.y + playerCol.max.y) &&
		Tmpl8::Game::tileMap->IsFree(playerPos.x + playerCol.max.x, playerPos.y + playerCol.max.y);
}
void MoveablePlayer::SetDashPos(Tmpl8::vec2& nextPos)
{
	linearT = timePassed / DASH_DURATION;
	nextPos = dashDir;
	nextPos *= MathFunctions::DashFunction(linearT);
}

void MoveablePlayer::ResetTriggers()
{

	canMove = false;
	hasChangedPos = false;
}

void MoveablePlayer::MovePlayer()
{
	if (canMove) {
		Collider c = (*col);
		if (Collider::TileMapInGameScreen(playerMovement, c * EDGE_DISTANCE))
			*pos = playerMovement;
		else {

			ClampTheMovementVector(c, playerMovement, *pos);
		}
	}
}

void MoveablePlayer::ClampTheMovementVector(const Collider& c, const Tmpl8::vec2 newVec, Tmpl8::vec2& originalVec, bool& changed)
{
	//try to move the player only on one axis
	Tmpl8::vec2 nextPos = (newVec - originalVec);
	if (CheckVecForOneDir(nextPos))
		return;
	Tmpl8::vec2 clampedOnX = Tmpl8::vec2(originalVec.x + nextPos.x / 2, originalVec.y);
	Tmpl8::vec2 clampedOnY = Tmpl8::vec2(originalVec.x, originalVec.y + nextPos.y / 2);
	if (Collider::TileMapInGameScreen(clampedOnX, c))
		originalVec = clampedOnX, changed = true;
	else if (Collider::TileMapInGameScreen(clampedOnY, c))
		originalVec = clampedOnY, changed = true;
}
bool MoveablePlayer::CheckVecForOneDir(Tmpl8::vec2& nextPos)
{

	return nextPos.x == 0 || nextPos.y == 0;


}
void MoveablePlayer::ClampTheMovementVector(const Collider& c, const Tmpl8::vec2 newVec, Tmpl8::vec2& originalVec)
{
	//try to move the player only on one axis
	Tmpl8::vec2 nextPos = newVec - originalVec;

	if (CheckVecForOneDir(nextPos))
		return;
	Tmpl8::vec2 clampedOnX;
	Tmpl8::vec2 clampedOnY;
	if (tileDiagonalMoved) {
		clampedOnX = Tmpl8::vec2(originalVec.x + nextPos.x / 2, originalVec.y);
		clampedOnY = Tmpl8::vec2(originalVec.x, originalVec.y + nextPos.y / 2);
	}
	else {
		clampedOnX = Tmpl8::vec2(originalVec.x + nextPos.x, originalVec.y);
		clampedOnY = Tmpl8::vec2(originalVec.x, originalVec.y + nextPos.y);
	}
	if (Collider::TileMapInGameScreen(clampedOnX, c))
		originalVec = clampedOnX;
	else if (Collider::TileMapInGameScreen(clampedOnY, c))
		originalVec = clampedOnY;
}



