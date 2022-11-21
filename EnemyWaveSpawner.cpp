#include "EnemyWaveSpawner.h"
#include <iostream>
#include <fstream>

#include "EnemyHoarder.h"
#include "EnemyRunner.h"

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
	enemyPrototypes(),
	player(nullptr),
	indexOfEnemiesToSpawn(0)


{

}

void EnemyWaveSpawner::Init(Being* _player)
{
	indexOfEnemiesToSpawn = 0;
	player = _player;
	timer.Init(this, 1.0f);
	//enemy prototypes intialization
	EnemyInit();
	InitializeSpawners();
	ReadWaves();
}

void EnemyWaveSpawner::EnemyInit()
{
	for (size_t i = 0; i < updateObjects.size(); i++)
		if (updateObjects[i]->getUpdateable())
			delete updateObjects[i];
	updateObjects.clear();

	activeColliders.clear();
	for (int i = 0; i < poolOfHoarders.size(); i++)
		delete poolOfHoarders[i];
	poolOfHoarders.clear();
	for (int i = 0; i < poolOfRunners.size(); i++)
		delete poolOfRunners[i];
	poolOfHoarders.clear();
	Spawner::ResetExplosions();
	for (size_t i = 0; i < NUMBER_OF_ENEMIES; i++) {
		delete enemyPrototypes[i];
	}
	for (size_t i = 0; i < NUMBER_OF_ENEMIES; i++) {
		enemyPrototypes[i] = CreateEnemy(allEnemyTypes[i]);
	}
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
void EnemyWaveSpawner::Call()
{
	if (!startedWave)
		SpawnCurrentWave();
	else {
		//this must have a size that is bigger than 1
		vector<EnemySpawner*> possibleSpawners;
		CheckThePossibleSpawner(possibleSpawners);
		size_t indexOfSpawner = static_cast<size_t>(randomNumbers.RandomBetweenInts(0, static_cast<int>(possibleSpawners.size())));
		assert(possibleSpawners.size() > 0);
		assert(indexOfSpawner < possibleSpawners.size());
		SpawnEnemy(possibleSpawners[indexOfSpawner]->GetSpawnerPos(), enemiesToSpawn[indexOfEnemiesToSpawn]);
		std::cout << indexOfSpawner << " ";
		indexOfEnemiesToSpawn++;
		//spawned the last enemy of the wave
		if (enemiesToSpawn.size() == indexOfEnemiesToSpawn) {
			timer.isFinished = true;
			startedWave = false;
		}
	}
}

EnemyWaveSpawner::~EnemyWaveSpawner()
{
	for (size_t i = 0; i < NUMBER_OF_ENEMIES; i++) {

		delete enemyPrototypes[i];
	}

}
void EnemyWaveSpawner::PlayerTakesDamage(Enemy* enemy)
{
	player->TakeDamage(enemy->getDg());
}
void EnemyWaveSpawner::SpawnCurrentWave() {
	startedWave = true;
	int weight = waves[indexWave].weight;
	enemiesToSpawn.clear();
	vector<EnemyTypes>possibleEnemies;

	CheckThePossibleEnemies(weight, possibleEnemies);

	while (weight != 0 && possibleEnemies.size() != 0) {

		size_t index;
		//if there are more than one enemy
		if (possibleEnemies.size() > 1)
			index = static_cast<size_t>(randomNumbers.RandomBetweenInts(0, static_cast<int>(possibleEnemies.size())));
		else
			index = 0;
		std::cout << index << ' ';
		EnemyTypes type = possibleEnemies[index];
		enemiesToSpawn.push_back(type);

		weight -= enemyPrototypes[type]->getWeight();
		//recheck the possible enemies
		CheckThePossibleEnemies(weight, possibleEnemies);
	}
	//interval for spawninig
	timer.Init(this, SPAWNING_INTERVAL, true);
	//spawn enemies in the spawners' positons
	indexOfEnemiesToSpawn = 0;


	std::cout << "index done\n";
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

void EnemyWaveSpawner::CheckThePossibleSpawner(vector<EnemySpawner*>& possibleSpawner)
{
	possibleSpawner.clear();
	for (size_t i = 0; i < enemySpawners.size(); i++) {
		if (!Collider::InGameScreen(enemySpawners[i]->GetSpawnerPos()))
			possibleSpawner.push_back(enemySpawners[i]);
	}
}
void EnemyWaveSpawner::SpawnEnemy(vec2 pos, EnemyTypes enemy)
{
	//no more enemies of this type
	Enemy* enemyToSpawn = nullptr;
	switch (enemy)
	{
	case Hoarder:
		if (IsPoolEmpty(poolOfHoarders))
			CreateMoreEnemies(Hoarder);
		enemyToSpawn = poolOfHoarders[poolOfHoarders.size() - 1];
		poolOfHoarders.pop_back();
		break;
	case Runner:
		if (IsPoolEmpty(poolOfRunners))
			CreateMoreEnemies(Runner);
		enemyToSpawn = poolOfRunners[poolOfRunners.size() - 1];
		poolOfRunners.pop_back();
		break;
	default:
		break;
	}
	if (enemy == NUMBER_OF_ENEMIES)
		ThrowError("enemy spawner was not initialized");

	else if (enemyToSpawn) {
		activeColliders.push_back(enemyToSpawn->getColl());
		vec2 randomDir = GetDirDeviation();
		//set position to the spawner's

		enemyToSpawn->Init(PosDir{ pos ,randomDir });
		Game::Get().AddCollider(enemyToSpawn->getColl());
		Game::Get().AddMoveable(enemyToSpawn->getMoveable());

	}
}

void EnemyWaveSpawner::Render(Surface* screen)
{
	for (int i = 0; i < enemySpawners.size(); i++) {
		enemySpawners[i]->Render(screen);
		screen->Box(static_cast<int>(enemySpawners[i]->GetSpawnerPos().x), static_cast<int>(enemySpawners[i]->GetSpawnerPos().y), static_cast<int>(enemySpawners[i]->GetSpawnerPos().x) + 5, static_cast<int>(enemySpawners[i]->GetSpawnerPos().y) + 5, 0xFF0000);
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


void EnemyWaveSpawner::AddEnemyToPool(Enemy* enemy, bool isDead)
{
	enemy->SetActive(false);
	activeColliders.erase(remove(
		activeColliders.begin()
		, activeColliders.end(), enemy->getColl())
		, activeColliders.end());


	if (isDead) {
		notify(enemy->getScore(), Additive);
		if (activeColliders.size() == 0)
			SpawnCurrentWave();
	}
	switch (enemy->GetEnemyType())
	{
	case Hoarder:
		poolOfHoarders.push_back(enemy);
		break;
	case Runner:
		poolOfRunners.push_back(enemy);
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
}
void EnemyWaveSpawner::CreateMoreEnemies(EnemyTypes enemyType)
{
	Enemy* enemy = nullptr;
	switch (enemyType)
	{
		//add the json values t
	case Hoarder:
		enemy = enemyPrototypes[Hoarder]->clone();
		break;
	case Runner:
		enemy = enemyPrototypes[Runner]->clone();

		break;
	default:
		ThrowError("The creation of the enemy has failed");
		break;
	}

	updateObjects.push_back(enemy);

	AddEnemyToPool(enemy);
}

void EnemyWaveSpawner::ThrowError(const char* place) {
	char t[128];
	sprintf(t, "Invalid json: %s", place);
	NotifyUser(t);
}
