#include <Club.h>

Club::Club(Vec2 clubSize)
{
	size = clubSize;
	reach = 1.f * clubSize.x;
}
Club::~Club()
{}

void Club::Init(PhysicsWorld *world)
{
	//this->AddComponent(new Sprite("Placeholders/clubHolder.png"));
	//this->transform.SetPosition(cavemanColl->GetPosition());
	this->transform.ScaleToSize(size);
	this->AddComponent(new Rigidbody(*world, COLLIDER_BALL, size));
	this->AddTag("Club");
	clubColl = this->GetComponent<Rigidbody>();
	clubColl->SetPhysicsGroup(-1);
	clubColl->SetActive(false);
}

void Club::update(float dt, Vec2 cavemanPos)
{
	//Vec2 campos = uthEngine.GetWindow().GetCamera().GetPosition();
	if (timeFromHit > 0)
	{
		clubColl->SetPosition(cavemanPos + (hitDirection * reach));
		timeFromHit -= dt;
	}
	else if (timeFromHit <= 0)
	{
		timeFromHit = 0;
		clubColl->SetPosition(cavemanPos);
		if (clubColl->IsActive())
			clubColl->SetActive(false);
		//
	}
	else
		clubColl->SetPosition(cavemanPos);
}

void Club::Hit(Vec2 cavemanPosition, Vec2 tapPosition)
{
	Vec2 temp = tapPosition - cavemanPosition;
	temp.normalize();
	hitDirection = temp;
	timeFromHit = 0.5f;
	clubColl->SetActive(true);
}

void Club::HasHit()
{
	timeFromHit = 0;
}