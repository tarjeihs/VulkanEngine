#pragma once

class CRenderPipeline
{
public:
	virtual ~CRenderPipeline() = default;

	virtual void Init() = 0;
	virtual void Teardown() = 0;
};