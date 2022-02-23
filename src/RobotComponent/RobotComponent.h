#pragma once
#ifndef _ROBOTCOMPONENT_H_
#define _ROBOTCONPONENT_H_

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "../Shape/Shape.h"
#include "../Program/Program.h"
#include "../MatrixStack/MatrixStack.h"
using std::vector, std::shared_ptr;

class RobotComponent
{
	private:
		glm::vec3 par_translate;
		glm::vec3 angles;
		glm::vec3 mesh_translate;
		glm::vec3 mesh_scale;
		glm::vec3 select_scale_mult;
		shared_ptr<Shape> comp_shape;
		shared_ptr<Program> prog;
		double rot_amount;
		bool draw_joint;
		bool selected;
		bool rotate_auto;
	public:
		RobotComponent(glm::vec3 par_t, glm::vec3 ang, glm::vec3 mesh_t, glm::vec3 sc, shared_ptr<Shape> comp_type, shared_ptr<Program> program, bool do_joint);
		void draw(shared_ptr<MatrixStack> MV, shared_ptr<Shape> joint_type);
		vector<shared_ptr<RobotComponent>> children;
		void setSelected(bool sel) { selected = sel; };
		void setScaleMult(glm::vec3 scale_mult) { select_scale_mult = scale_mult; };
		void updateX(double delta) { angles.x += delta; };
		void updateY(double delta) { angles.y += delta; };
		void updateZ(double delta) { angles.z += delta; };
		void make_rotate(bool rot) { rotate_auto = rot; }
};


#endif
