#include "RobotComponent.h"
#include <GLFW/glfw3.h>
#include <iostream>

RobotComponent::RobotComponent(glm::vec3 par_t, glm::vec3 ang, glm::vec3 mesh_t, glm::vec3 sc, shared_ptr<Shape> comp_type, shared_ptr<Program> program, bool do_joint) {
	par_translate = par_t;
	angles = ang;
	mesh_translate = mesh_t;
	mesh_scale = sc;
	comp_shape = comp_type;
	prog = program;
	draw_joint = do_joint;
	selected = false;
	rotate_auto = false;
}

void RobotComponent::draw(shared_ptr<MatrixStack> MV, shared_ptr<Shape> joint_type) {
	MV->pushMatrix();
		MV->translate(par_translate);
		// rot x
		MV->rotate(angles.x, 1.0, 0.0, 0.0);
		// rot y
		MV->rotate(angles.y, 0.0, 1.0, 0.0);
		// rot z
		MV->rotate(angles.z, 0.0, 0.0, 1.0);
		MV->pushMatrix();
		if(rotate_auto) {
			double t = glfwGetTime();
			MV->rotate(t, 1.0, 0.0, 0.0);
		}
		if(draw_joint) {
			glUniformMatrix4fv(prog->getUniform("MV"), 1, false, &MV->topMatrix()[0][0]);
			joint_type->draw(prog);
		}
		MV->translate(mesh_translate);
		MV->scale(mesh_scale);
		if(selected) {
			MV->scale(select_scale_mult);
		}
		glUniformMatrix4fv(prog->getUniform("MV"), 1, false, &MV->topMatrix()[0][0]);
		comp_shape->draw(prog);
		MV->popMatrix();
		for(unsigned int i = 0; i < children.size(); i++) {
			children[i]->draw(MV, joint_type);
		}
	MV->popMatrix();
}
