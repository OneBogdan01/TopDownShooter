#include "EnemyHoarder.h"
#include "MathFunctions.h"
EnemyHoarder::EnemyHoarder(PosDir posDir, Tmpl8::Sprite* sprite, EnemyWaveSpawner* spawner) :
	Enemy(posDir.pos, sprite, spawner),
	randomNumbers()
{
	enemyType = Hoarder;
	//json magic
	score = 10;
	dg = DG;
	col = (new Collider(COL_MIN, COL_MAX, &pos));
	mover = new MoveToADirection(&pos, &dir, col, this, SPEED);
	//as a getter for the base class
	move = mover;

	attack = Timer(this, TIME_TO_ATTACK, true);
	rotate = Timer();
	rot = new EnemyRotator(&pos, &dir, rVar, &frame, mover, spawner);

	Init(posDir);
}



EnemyHoarder::~EnemyHoarder()
{
	delete mover;
	delete rot;
}

void EnemyHoarder::Update(float deltaTime)
{
	if (!getUpdateable())
		return;

	rotate.Update(deltaTime);
	//marked by collision
	if (col->toDeactivate) {
		//take damage from the collision if it is from the projectile or block
		if (!spawner->IsEnemy(col->collision)) {
			TakeDamage(DG_TO_TAKE);
		}
		else {
			//interacting with enemy 

		}
		col->toDeactivate = false;

	}
	else {
		mover->Update(deltaTime);

		dist = MathFunctions::GetDistanceSqr(pos, spawner->GetPlayerPos());

		if (dist > MAX_DISTANCE_TO_PLAYER) {
			//not in range
			InRangeToAtack = false;
		}
		else if (dist < MAX_DISTANCE_TO_ATTACK) {
			//in range to atack player
			attack.Update(deltaTime);
			InRangeToAtack = true;
		}
		else
		{
			InRangeToAtack = false;
		}

	}


}

void EnemyHoarder::Render(Tmpl8::Surface* screen)
{
	if (!getRenderable())
		return;
	sprite->SetFrame(frame);
	sprite->Draw(screen, static_cast<int>(static_cast<int>(pos.x - rVar.SPRITE_OFFSET / 2)), static_cast<int>(pos.y - rVar.SPRITE_OFFSET / 2));
	screen->Box(static_cast<int>(pos.x - rVar.SPRITE_OFFSET / 2),
		static_cast<int>(pos.y - rVar.SPRITE_OFFSET / 2),
		static_cast<int>(pos.x + rVar.SPRITE_OFFSET / 2),
			static_cast<int>(pos.y + rVar.SPRITE_OFFSET / 2), 0xffff);

}



void EnemyHoarder::Init(PosDir posDir)
{
	SetActive(true);
	pos = posDir.pos;
	dir = posDir.dir;
	hp = 100;
	mover->SetSpeed(SPEED + randomNumbers.RandomBetweenFloats(-30, 100));
	rotate.Init(rot, randomNumbers.RandomBetweenFloats(0.1f, 0.9f), true);
	rot->Call();
}



void EnemyHoarder::ResetEnemy()
{

	spawner->AddEnemyToPool(this, true);
	spawner->SpawnExplosions(pos);
}

void EnemyHoarder::Call()
{
	//ready to atack and in range
	if (attack.FinishedLoop() && InRangeToAtack) {
		spawner->PlayerTakesDamage(this);
		attack.ResetVar();
		//moves a bit after atacking
		ToMove = true;
	}
	else if (mover->colToReflectFrom != NULL) {
		Collider c = *mover->colToReflectFrom;


		rot->Reflect(Collider::GetNormal(c, *col));

		mover->colToReflectFrom = NULL;
	}
	//out of bounds
	else {
		rot->Reflect(Collider::GetNormalEdgeScreen(mover->nextP, *mover->getColl()));
	}

}

void EnemyHoarder::Die()
{
	ResetEnemy();
}

Enemy* EnemyHoarder::clone()
{
	return new EnemyHoarder(PosDir(pos, dir), sprite, spawner);
}
