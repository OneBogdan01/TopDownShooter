#include "CollisionDetection.h"


CollisionDetection::CollisionDetection(vector<Collider*>& projectiles, vector<Collider*>& enemies) :
	Projectiles(&projectiles),
	Enemies(&enemies)

{
	timer = new Timer(this, .05f, true);
}



CollisionDetection::~CollisionDetection()
{
	delete timer;

}



void CollisionDetection::DetectCollisions()
{

	vector <Collider*> allPairs;
	vector <Collider*> activeIntervals;


	vector<Collider*> cols;
	//add enemies and projectiles
	for (int i = 0; i < Projectiles->getCount(); i++)
		cols.push_back(Projectiles->get(i));
	for (int i = 0; i < Enemies->getCount(); i++)
		cols.push_back(Enemies->get(i));
	//sort on x axis
	if (cols.getCount() == 0)
		return;
	mergeSort.Init(&cols);
	mergeSort.Sort(0, cols.getCount() - 1);

	for (int i = 0; i < cols.getCount(); i++)
	{
		Collider* a = cols.get(i);

		for (int j = 0; j < activeIntervals.getCount(); j++) {
			//possible collision
			Collider* b = activeIntervals.get(j);

			if (Collider::CollidesX(a->At(*a->pos),
				b->At(*b->pos)))
			{
				//check if there is a collision
				if (Collider::CollidesY(a->At(*a->pos),
					b->At(*b->pos))) {
					allPairs.push_back(a);
					allPairs.push_back(b);
				}
			}
			else {
				//update the interval if no collision

				activeIntervals.removeAtIndex(j);

				j--;
			}
		}
		activeIntervals.push_back(a);
	}

	for (int i = 0; i < allPairs.getCount(); i += 2) {
		allPairs[i]->toDeactivate = true;
		allPairs[i]->collision = allPairs[i + 1];
		allPairs[i + 1]->toDeactivate = true;
		allPairs[i + 1]->collision = allPairs[i];

	}

}

void CollisionDetection::Update(float deltaTime)
{
	timer->Update(deltaTime);
}


void CollisionDetection::Call()
{
	DetectCollisions();
}

