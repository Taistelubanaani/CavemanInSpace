#include "Caveman.h"

void Caveman::Init(PhysicsWorld *world)
{
	speed = 4;
	this->AddComponent(new AnimatedSprite(uthRS.LoadTexture("caveman_animations.png"), 26, Vec2(119, 132), 5, 0/*,false,false*/));
	Vec2 originOffset = Vec2(-(this->transform.GetSize().x) / 6, (this->transform.GetSize().y) / 7);
	this->transform.SetOrigin(originOffset);
	this->transform.SetPosition(0, 0);
	this->AddComponent(new Rigidbody(*world, COLLIDER_BALL, Vec2(85, 1)));
	cavemanColl = this->GetComponent<Rigidbody>();
	cavemanColl->SetVelocity(pmath::Vec2(0, 0));
	cavemanColl->SetPhysicsGroup(-1);
	this->AddTag("Caveman");
	this->GetComponent<AnimatedSprite>()->ChangeAnimation(0, 1);
}


void Caveman::Hit(const Vec2& hitPoint, bool isDying)
{
	if (animTime <= 0 && !isDying)
	{
		this->GetComponent<AnimatedSprite>()->ChangeAnimation(0, 10, 0, 10);
		animTime = 1.f;
	}

	// Sets the caveman to turn towards the click
	if (hitPoint.length() != 0)
	{
		rotate = true;
		targetRotation = atan2(hitPoint.x - transform.GetPosition().x, (hitPoint.y - transform.GetPosition().y) * -1);
		targetRotation *= 180 / pi;
		targetRotation -= 90;
		GetComponent<Rigidbody>("Rigidbody")->SetAngularVelocity(0);

		if (targetRotation > 180) targetRotation -= 360;
		else if (targetRotation < -180) targetRotation += 360;

		if (abs(targetRotation - transform.GetRotation()) > 180)
		{
			if (transform.GetRotation() > 0)
				rotatePositive = true;
			else
				rotatePositive = false;
		}
		else
		{
			if (targetRotation > transform.GetRotation())
				rotatePositive = true;
			else
				rotatePositive = false;
		}
	}
}
void Caveman::Die()
{
	this->GetComponent<AnimatedSprite>()->ChangeAnimation(10, 16, 0, 10);
	isDead = true;
}
// Used for both input types!
void Caveman::ChangeDirectionMouse(pmath::Vec2 arrowDirection, bool strongpull)
{
	// Calculates the vector between touch position and caveman's position.
	//pmath::Vec2 temp = (-1.f * pullPosition) + (uthEngine.GetWindow().GetSize() * 0.5f);
	//temp.normalize();
	if (strongpull)
		speed = 7;
	else
		speed = 4;

	cavemanColl->SetVelocity(arrowDirection * speed);
}

// not used anywhere.
void Caveman::ChangeDirectionTouch(pmath::Vec2 startPosition, pmath::Vec2 endPosition)
{
	pmath::Vec2 temp = cavemanColl->GetPosition() - endPosition;
	speed = 5; // if resolution problems, calc as percentage of screen size
	temp.normalize();
	cavemanColl->SetVelocity(temp * speed);
}

void Caveman::update(float dt)
{
	if (!isDead)
	{
		if (animTime > 0) animTime -= dt;
		else  this->GetComponent<AnimatedSprite>()->ChangeAnimation(0, 1);
	}

	// Turns the caveman gradually towards the hit position.

	if (transform.GetRotation() > 180)
		GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() - 360);
	if (transform.GetRotation() < -180)
		GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() + 360);

	if (rotate)
	{
		if (abs(targetRotation - transform.GetRotation()) < 4)
		{
			rotate = false;

			//if (rotatePositive) GetComponent<Rigidbody>("Rigidbody")->SetAngularVelocity(-0.2f);
			//else GetComponent<Rigidbody>("Rigidbody")->SetAngularVelocity(0.2f);
		}
		else
		{
			if (rotatePositive)
				GetComponent<Rigidbody>()->Rotate(150 * dt);
			else
				GetComponent<Rigidbody>()->Rotate(-150 * dt);
		}
	}
}

Caveman::Caveman()
	: rotate(false),
	rotatePositive(false),
	isDead(false),
	timeFromHit(0),
	animTime(0),
	targetRotation(0),
	hitDirection(0,0)
{
}
Caveman::~Caveman()
{
}

// The jolly ol' rrrotation
//
//if (rotate)
//{
//	if (abs(targetRotation - transform.GetRotation()) < 4)
//	{
//		rotate = false;
//
//		if (rotatePositive) GetComponent<Rigidbody>("Rigidbody")->SetAngularVelocity(-0.2f);
//		else GetComponent<Rigidbody>("Rigidbody")->SetAngularVelocity(0.2f);
//	}
//	else
//	{
//		if (abs(targetRotation - transform.GetRotation()) < 180)
//		{
//			if (rotatePositive)
//				GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() + (abs(targetRotation - transform.GetRotation()) * 3 * dt));
//			else
//				GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() + (abs(targetRotation - transform.GetRotation()) * -3 * dt));
//		}
//		else
//		{
//			if (rotatePositive)
//				GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() + (360 - abs(targetRotation - transform.GetRotation())) * 3 * dt);
//			else
//				GetComponent<Rigidbody>("Rigidbody")->SetAngle(transform.GetRotation() + (360 - abs(targetRotation - transform.GetRotation())) * -3 * dt);
//		}
//	}