#include "build-density.h"

#include "../gl.h"


DensityBuilder::DensityBuilder(DensityParams params, uint32_t shader) : mParams(params), mShader(shader)
{
	gl::createQuery(&mTimerQuery, 1);
}

void DensityBuilder::generate(uint32_t texture, uint32_t width, uint32_t height, uint32_t depth, glm::ivec3 offset)
{
	glBeginQuery(GL_TIME_ELAPSED, mTimerQuery);
	uint32_t sizeX = (width + 7 )  / 8;
	uint32_t sizeY = (height + 7 ) / 8;
	uint32_t sizeZ = (depth + 7 )  / 8;

	glUseProgram(mShader);

	glm::vec3 values{ offset };
	glUniform3fv(1, 1, &values[0]);
	glBindImageTexture(0, texture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(sizeX, sizeY, sizeZ);

	glUseProgram(0);

	glEndQuery(GL_TIME_ELAPSED);

	// Wait until the query results are available
	int done = 0;
	while (!done) {
		glGetQueryObjectiv(mTimerQuery,
			GL_QUERY_RESULT_AVAILABLE,
			&done);
	}

	// get the query results
	uint64_t elapsedTime = 0;
	glGetQueryObjectui64v(mTimerQuery, GL_QUERY_RESULT, &elapsedTime);

	timeToGenerateDensity = elapsedTime * 1e-6f;
	std::cout << "\033[31;1;4mDensity Texture Generation[" << timeToGenerateDensity << "ms ]\033[0m" << std::endl;

}
