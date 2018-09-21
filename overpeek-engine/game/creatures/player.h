#pragma once

#define PLAYER_WIDTH 0.075
#define PLAYER_HEIGHT 0.075

class Player {
private:
	float m_x, m_y;
	float m_vel_x, m_vel_y;
	float m_acc_x, m_acc_y;

public:
	Player(float x, float y);

	void render();
	void update();

	inline void setX(float value) { m_x = value; }
	inline void setVelX(float value) { m_vel_x = value; }
	inline void setAccX(float value) { m_acc_x = value; }
	inline void setY(float value) { m_y = value; }
	inline void setVelY(float value) { m_vel_y = value; }
	inline void setAccY(float value) { m_acc_y = value; }

	inline float getX() { return m_x; }
	inline float getVelX() { return m_vel_x; }
	inline float getAccX() { return m_acc_x; }
	inline float getY() { return m_y; }
	inline float getVelY() { return m_vel_y; }
	inline float getAccY() { return m_acc_y; }

};