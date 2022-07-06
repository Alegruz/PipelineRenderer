/*+===================================================================
  File:      SHADER.H

  Summary:   Shader header file contains declarations of Shader
             class used for the lab samples of Game Graphics
             Programming course.

  Classes: Shader

  ?2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "pch.h"

namespace pr
{
    constexpr PCWSTR VS_VERTEX_PCN = L"VSPCN.cso";
    constexpr PCWSTR PS_DEPTH = L"PSDepth.cso";
    constexpr PCWSTR PS_NORMAL = L"PSNormal.cso";

    //class Shader
    //{
    //public:
    //    Shader() = delete;
    //    Shader(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR pszShaderModel);
    //    Shader(const Shader& other) = delete;
    //    Shader(Shader&& other) = delete;
    //    Shader& operator=(const Shader& other) = delete;
    //    Shader& operator=(Shader&& other) = delete;
    //    virtual ~Shader() = default;

    //    virtual HRESULT Initialize(_In_ ID3D11Device* pDevice) = 0;
    //    PCWSTR GetFileName() const;

    //protected:
    //    HRESULT compile(_Outptr_ ID3DBlob** ppOutBlob);

    //    PCWSTR m_pszFileName;
    //    PCSTR m_pszEntryPoint;
    //    PCSTR m_pszShaderModel;
    //};
}