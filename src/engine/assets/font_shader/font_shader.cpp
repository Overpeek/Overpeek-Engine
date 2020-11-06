#include "font_shader.hpp"

#include "shader.vert.hpp"
#include "shader.frag.hpp"

#include "engine/engine.hpp"
#include "engine/graphics/interface/shader.hpp"
#include "engine/graphics/interface/instance.hpp"
#include "engine/utility/formatted_error.hpp"



namespace oe::assets {

	constexpr char font_shader_name[] = "asset:font_shader";
	


	FontShader::FontShader()
	{
		m_shader = oe::graphics::Shader({
			font_shader_name,
			{
				ShaderStageInfo {
					shader_stages::vertex_shader,
					shader_vert_gl,
					{}
				},
				ShaderStageInfo {
					shader_stages::fragment_shader,
					shader_frag_gl,
					{}
				}
			}
		});
	}

	void FontShader::bind() const
	{
		m_shader->bind();
		oe::Engine::getSingleton().polygonMode(oe::polygon_mode::fill);
	}

	void FontShader::unbind() const
	{
		m_shader->unbind();
		oe::Engine::getSingleton().polygonMode(oe::polygon_mode::fill);
	}

	void FontShader::setProjectionMatrix(const glm::mat4& pr_mat)
	{
		if(m_pr_mat == pr_mat) return;
		m_pr_mat = pr_mat;
		updateMVP();
	}

	void FontShader::setViewMatrix(const glm::mat4& vw_mat)
	{
		if(m_vw_mat == vw_mat) return;
		m_vw_mat = vw_mat;
		updateMVP();
	}
		
	void FontShader::setModelMatrix(const glm::mat4& ml_mat)
	{
		if(m_ml_mat == ml_mat) return;
		m_ml_mat = ml_mat;
		updateMVP();
	}

	void FontShader::updateMVP() const
	{
		m_shader->setUniform("mvp_matrix", m_pr_mat * m_vw_mat * m_ml_mat);
	}

	void FontShader::setColor(const oe::color& color) const
	{
		m_shader->setUniform("u_color", color);
	}

	void FontShader::setTexture(bool use) const
	{
		m_shader->setUniform("u_usetex", (int)use);
	}

	void FontShader::setSDF(bool use) const
	{
		m_shader->setUniform("u_usesdf", (int)use);
	}

	void FontShader::setWidth(float width) const
	{
		m_shader->setUniform("u_sdf_width", width);
	}

	void FontShader::setEdge(float edge) const
	{
		m_shader->setUniform("u_sdf_edge", edge);
	}

	void FontShader::setOutlineWidth(float width) const
	{
		m_shader->setUniform("u_sdf_outline_width", width);
	}

	void FontShader::setOutlineEdge(float edge) const
	{
		m_shader->setUniform("u_sdf_outline_edge", edge);
	}

	void FontShader::setOutlineColor(const oe::color& c) const
	{
		m_shader->setUniform("u_sdf_outline_color", c);
	}

}