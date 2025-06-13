#include "Building.h"

void Building::DrawBuilding(int squareWidth)
{
	raylib::Rectangle sourceRectangle(0, 0, (float)texture->GetWidth(), (float)texture->GetHeight());
	raylib::Rectangle destinationRectangle((float)originPosition.x, (float)originPosition.y, (float)squareWidth, (float)squareWidth);
	Vector2 origin = { 0, 0 };
	texture->Draw(sourceRectangle, destinationRectangle, origin, 0.0f, raylib::Color::White());
}