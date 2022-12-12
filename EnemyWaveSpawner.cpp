#include "EnemyWaveSpawner.h"
#include <iostream>
#include <fstream>

#include "EnemyHoarder.h"
#include "EnemyRunner.h"
#include "EnemyShooter.h"
#include "EnemyShielder.h"

#include "game.h"
using namespace std;
using namespace Tmpl8;
namespace Tmpl8 {
	void NotifyUser(const char* s);
}
EnemyWaveSpawner::EnemyWaveSpawner()
	:
	Spawner("assets/OriginalAssets/smoke.tga"),
	hoarderSprite(new Surface("assets/OriginalAssets/phaser.tga"), 16),
	runnerSprite(new Surface("assets/OriginalAssets/sniper.tga"), 32),
	shooterSprite(new Surface("assets/OriginalAssets/sniper.tga"), 32),
	shielderSprite(new Surface("assets/OriginalAssets/sniper.tga"), 32),
	indexOfEnemiesToSpawn(0)
{

}

void EnemyWaveSpawner::Init()
{
	minimumProjectiles = 0;
	indexOfEnemiesToSpawn = 0;
	firstWave = true;
	timer.Init(bind(&EnemyWaveSpawner::SpawnCurrentWave, this), 1.0f);
	playerDistanceSqr = Game::Get().getPlayer().GetHalfCollider();
	//square it
	playerDistanceSqr *= playerDistanceSqr;
	//enemy prototypes intialization
	EnemyInit();
	InitializeSpawners();
	ReadWaves();
}

void EnemyWaveSpawner::EnemyInit()
{
	ClearVecOfPointers();

	for (size_t i = 0; i < (NUMBER_OF_ENEMIES); i++) {
		enemyPrototypes[i] = CreateEnemy(static_cast<EnemyTypes>(i));
	}
}

void EnemyWaveSpawner::ClearVecOfPointers()
{
	for (size_t i = 0; i < updateObjects.size(); i++)
		if (updateObjects[i]->getUpdateable())
			delete updateObjects[i];
	updateObjects.clear();

	activeColliders.clear();

	ClearPoolOfEnemies(poolOfHoarders);
	ClearPoolOfEnemies(poolOfRunners);
	ClearPoolOfEnemies(poolOfShooters);
	ClearPoolOfEnemies(poolOfShielders);

	Spawner::ResetExplosions();

	for (size_t i = 0; i < (NUMBER_OF_ENEMIES); i++) {
		delete enemyPrototypes[i];
	}
}


void EnemyWaveSpawner::ClearPoolOfEnemies(vector<Enemy*>& pool)
{
	for (int i = 0; i < pool.size(); i++)
		delete pool[i];
	pool.clear();
}

void EnemyWaveSpawner::InitializeSpawners()
{
	//place the spawners in the four corners of the map
	vec2 center = vec2(ScreenWidth / 2, ScreenHeight / 2);
	float xOffset = SPAWNERS_XPOS_MULTIPLIERS * ScreenWidth;
	float yOffset = SPAWNERS_YPOS_MULTIPLIERS * ScreenHeight;

	for (auto p : enemySpawners)
		delete p;
	enemySpawners.clear();

	enemySpawners.push_back(new EnemySpawner(center + vec2(xOffset, yOffset), this, explosionSprite));
	enemySpawners.push_back(new EnemySpawner(center + vec2(-xOffset, yOffset), this, explosionSprite));
	enemySpawners.push_back(new EnemySpawner(center + vec2(xOffset, -yOffset), this, explosionSprite));
	enemySpawners.push_back(new EnemySpawner(center + vec2(-xOffset, -yOffset), this, explosionSprite));
}
void EnemyWaveSpawner::ReadWaves()
{
	std::ifstream f("json/enemy_waves.json");
	json  wavesInput = json::parse(f);
	std::cout << "json parsed successfully\n";
	wavesCount = wavesInput["waves"].size();

	for (int i = 0; i < wavesCount; i++) {
		waves[i].weight = wavesInput["waves"][i].at("weight");

		//check if weight is valid
		if (waves[i].weight <= 0)
			ThrowError("weight must be a positive integer");
		for (int j = 0; j < wavesInput["waves"][i].at("enemy_types").size(); j++) {
			//check if enum is valid and avoid duplicates
			EnemyTypes t = ConvertToEnum(wavesInput["waves"][i].at("enemy_types")[j]);
			if (find(waves[i].enemiesInWave.begin(), waves[i].enemiesInWave.end(), t) == waves[i].enemiesInWave.end())
				waves[i].enemiesInWave.push_back(t);
		}
	}

}
//spawns enemies at an inverval for random numbers to use time as a seed
void EnemyWaveSpawner::SpawnCurrentWave()
{
	if (!startedWave)
		GetEnemiesForCurrentWave();
	else {
		//this must have a size that is bigger than 1
		vector<EnemySpawner*> possibleSpawners;
		CheckTheOffscreenSpawners(possibleSpawners);
		size_t indexOfSpawner = static_cast<size_t>(randomNumbers.RandomBetweenInts(0, static_cast<int>(possibleSpawners.size())));
		//error if the asumption about spawners is not right
		assert(possibleSpawners.size() > 0 &&
			indexOfSpawner < possibleSpawners.size());

		SpawnEnemy(PosDir{ possibleSpawners[indexOfSpawner]->GetSpawnerPos() ,0 }, enemiesToSpawn[indexOfEnemiesToSpawn]);
		std::cout << "Spawned enemies:" << (enemiesToSpawn[indexOfEnemiesToSpawn]) << '\n';
		indexOfEnemiesToSpawn++;
		//spawned the last enemy of the wave
		if (enemiesToSpawn.size() == indexOfEnemiesToSpawn) {
			if (!firstWave) {
				//notify the score 
				//multiply the score if the player was not hit the previous wave 
				if (!playerHasTakenDamage) {
					notify(1, EventType::BonusConditions);
					std::cout << "Double points for health\n";
				}
				else {
					playerHasTakenDamage = false;
				}

			}

			firstWave = false;

			timer.isFinished = true;
			startedWave = false;
		}
	}
}

EnemyWaveSpawner::~EnemyWaveSpawner()
{
	ClearVecOfPointers();

}
void EnemyWaveSpawner::PlayerTakesDamage(Enemy* enemy)
{
	notify(enemy->getDg(), EventType::PlayerTakesDamage);
	playerHasTakenDamage = true;
}
void EnemyWaveSpawner::GetEnemiesForCurrentWave() {
	startedWave = true;
	int weight = waves[indexWave].weight;
	enemiesToSpawn.clear();
	vector<EnemyTypes>possibleEnemies;

	CheckThePossibleEnemies(weight, possibleEnemies);
	if (possibleEnemies.size() == 0)
		ThrowError("Too small weight for spawning any enemies");
	while (weight != 0 && possibleEnemies.size() != 0) {

		size_t index;
		//if there are more than one enemy
		if (possibleEnemies.size() > 1)
			index = static_cast<size_t>(randomNumbers.RandomBetweenInts(0, static_cast<int>(possibleEnemies.size())));
		else
			index = 0;
		EnemyTypes type = possibleEnemies[index];
		enemiesToSpawn.push_back(type);

		weight -= enemyPrototypes[type]->getWeight();
		//recheck the possible enemies
		CheckThePossibleEnemies(weight, possibleEnemies);
	}
	//interval for spawning
	timer.Init(bind(&EnemyWaveSpawner::SpawnCurrentWave, this), SPAWNING_INTERVAL, true); //spawning interval could be fetched from the json per wave
	//spawn enemies in the spawners' positons
	indexOfEnemiesToSpawn = 0;


}
void EnemyWaveSpawner::CheckThePossibleEnemies(size_t weight, vector<EnemyTypes>& possibleEnemies)
{
	possibleEnemies.clear();
	for (size_t i = 0; i < waves[indexWave].enemiesInWave.size(); i++) {
		//checks if the enemy weight is bigger the the weight of the entire wave
		if (enemyPrototypes[waves[indexWave].enemiesInWave[i]]->getWeight() <= weight)
			possibleEnemies.push_back(waves[indexWave].enemiesInWave[i]);
	}
}

void EnemyWaveSpawner::CheckTheOffscreenSpawners(vector<EnemySpawner*>& possibleSpawner)
{
	possibleSpawner.clear();
	for (size_t i = 0; i < enemySpawners.size(); i++) {
		if (!Collider::InGameScreen(enemySpawners[i]->GetSpawnerPos()))
			possibleSpawner.push_back(enemySpawners[i]);
	}
}
void EnemyWaveSpawner::SpawnEnemy(PosDir posDir, EnemyTypes enemy)
{
	Enemy* enemyToSpawn = nullptr;
	switch (enemy)
	{
	case Hoarder:
		if (IsPoolEmpty(poolOfHoarders))
			CreateMoreEnemies(enemy);
		enemyToSpawn = poolOfHoarders[poolOfHoarders.size() - 1];
		poolOfHoarders.pop_back();
		break;
	case Runner:
		if (IsPoolEmpty(poolOfRunners))
			CreateMoreEnemies(enemy);
		enemyToSpawn = poolOfRunners[poolOfRunners.size() - 1];
		poolOfRunners.pop_back();
		break;
	case Shooter:
		if (IsPoolEmpty(poolOfShooters))
			CreateMoreEnemies(enemy);
		enemyToSpawn = poolOfShooters[poolOfShooters.size() - 1];
		poolOfShooters.pop_back();
		break;
	case Shielder:
		if (IsPoolEmpty(poolOfShielders))
			CreateMoreEnemies(enemy);
		enemyToSpawn = poolOfShielders[poolOfShielders.size() - 1];
		poolOfShielders.pop_back();
		break;
	default:
		ThrowError("could not spawn enemies");
		break;
	}

	if (enemyToSpawn) {
		if (enemyToSpawn->getEnemyType() != EnemyTypes::Runner) {//runner has a predefined lifetime
			minimumProjectiles += enemyToSpawn->getMaxHp() / enemyToSpawn->getDgToTake();
			std::cout << "min proje" << minimumProjectiles << "\n";
		}
		activeColliders.push_back(enemyToSpawn->getColl());
		//set position to the spawner's

		enemyToSpawn->Init(posDir);
		Game::Get().AddCollider(enemyToSpawn->getColl());
		Game::Get().AddMoveable(enemyToSpawn->getMoveable());

	}
}

void EnemyWaveSpawner::Render(Surface* screen)
{
	for (int i = 0; i < enemySpawners.size(); i++) {
		enemySpawners[i]->Render(screen);
#ifdef _DEBUG
		screen->Box(static_cast<int>(enemySpawners[i]->GetSpawnerPos().x), static_cast<int>(enemySpawners[i]->GetSpawnerPos().y), static_cast<int>(enemySpawners[i]->GetSpawnerPos().x) + 5, static_cast<int>(enemySpawners[i]->GetSpawnerPos().y) + 5, 0xFF0000);
#endif
	}

	for (int i = 0; i < updateObjects.size(); i++)
		updateObjects[i]->Render(screen);
}

void EnemyWaveSpawner::Update(float deltaTime)
{
	timer.Update(deltaTime);
	for (int i = 0; i < enemySpawners.size(); i++)
		enemySpawners[i]->Update(deltaTime);

	for (int i = 0; i < updateObjects.size(); i++)
		updateObjects[i]->Update(deltaTime);
}


void EnemyWaveSpawner::AddEnemyToPool(Enemy* enemy, bool getPoints)
{
	enemy->SetActive(false);
	activeColliders.erase(remove(
		activeColliders.begin()
		, activeColliders.end(), enemy->getColl())
		, activeColliders.end());


	if (getPoints) {
		notify(enemy->getScore(), EventType::EnemyDeath);
	}
	if (activeColliders.size() == 0) {
		if (enemiesToSpawn.size() == indexOfEnemiesToSpawn) {
			//number of minimum projectiles

			notify(minimumProjectiles, EventType::EndOfAWave);
			minimumProjectiles = 0;
		}
		GetEnemiesForCurrentWave();
	}
	switch (enemy->getEnemyType())
	{
	case Hoarder:
		poolOfHoarders.push_back(enemy);
		break;
	case Runner:
		poolOfRunners.push_back(enemy);
		break;
	case Shooter:
		poolOfShooters.push_back(enemy);
		break;
	case Shielder:
		poolOfShielders.push_back(enemy);
		break;
	default:
		break;
	}
	//game wiring
	Game::Get().RemoveCollider(enemy->getColl());
	Game::Get().RemoveMoveable(enemy->getMoveable());
}
Enemy* EnemyWaveSpawner::CreateEnemy(EnemyTypes enemyType) {

	std::ifstream f;
	json enemyJson;
	Enemy* enemy = nullptr;

	switch (enemyType)
	{
	case Hoarder:
		f.open("json/Hoarder.json");
		enemy = new EnemyHoarder(PosDir(vec2(0), vec2(0)), &hoarderSprite, this);
		enemyJson = json::parse(f);

		SetJsonValues(enemy, enemyJson);

		break;
	case Runner:
		f.open("json/Runner.json");

		enemy = new EnemyRunner(PosDir(vec2(0), vec2(0)), &runnerSprite, this);
		enemyJson = json::parse(f);

		SetJsonValues(enemy, enemyJson);
		break;
	case Shooter:
		f.open("json/Shooter.json");

		enemy = new EnemyShooter(PosDir(vec2(0), vec2(0)), &shooterSprite, this);
		enemyJson = json::parse(f);

		SetJsonValues(enemy, enemyJson);
		break;
	case Shielder:
		f.open("json/Shielder.json");

		enemy = new EnemyShielder(PosDir(vec2(0), vec2(0)), &shielderSprite, this);
		enemyJson = json::parse(f);

		SetJsonValues(enemy, enemyJson);
		break;
	default:
		ThrowError("The creation of the enemy has failed");
		break;
	}
	return enemy;
}
void EnemyWaveSpawner::SetJsonValues(Enemy* enemy, json& enemyJson)
{
	enemy->setDg(enemyJson["dg"]);
	enemy->setHp(enemyJson["hp"]);
	enemy->setScore(enemyJson["score"]);
	enemy->setWeight(enemyJson["weight"]);
	enemy->setDgToTake(enemyJson["dgTake"]);
}
void EnemyWaveSpawner::CreateMoreEnemies(EnemyTypes enemyType)
{
	Enemy* enemy = nullptr;

	if (enemyType < NUMBER_OF_ENEMIES && enemyType >= Hoarder)
		enemy = enemyPrototypes[enemyType]->clone();
	else
		ThrowError("The creation of the enemy has failed");
	updateObjects.push_back(enemy);

	AddEnemyToPool(enemy);
}

void EnemyWaveSpawner::ThrowError(const char* place) {
	char t[128];
	sprintf(t, "Invalid json: %s", place);
	NotifyUser(t);
}
