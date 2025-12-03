$input v_color0, v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(textureColor, 0);

void main()
{
	gl_FragColor = v_color0 * texture2D(textureColor, v_texcoord0);
}