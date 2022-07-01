#include "pch.h"

#include "Camera/Camera.h"

namespace pr
{
    Camera::Camera(_In_ const XMVECTOR& position)
        : m_Rotation()
        , m_View()
        , m_CameraForward(DEFAULT_FORWARD)
        , m_CameraRight(DEFAULT_RIGHT)
        , m_CameraUp(DEFAULT_UP)
        , m_Eye(position)
        , m_At(position + m_CameraForward)
        , m_Up(m_CameraUp)
        , m_Yaw(0.0f)
        , m_Pitch(0.0f)
        , m_MoveLeftRight(0.0f)
        , m_MoveBackForward(0.0f)
        , m_MoveUpDown(0.0f)
        , m_TravelSpeed(12.0f)
        , m_RotationSpeed(0.004f)
    {
        m_View = XMMatrixLookAtLH(m_Eye, m_At, m_Up);
    }

    const XMVECTOR& Camera::GetEye() const
    {
        return m_Eye;
    }

    const XMVECTOR& Camera::GetAt() const
    {
        return m_At;
    }

    const XMVECTOR& Camera::GetUp() const
    {
        return m_Up;
    }

    constexpr const XMMATRIX& Camera::GetView() const noexcept
    {
        return m_View;
    }

    //ComPtr<ID3D11Buffer>& Camera::GetConstantBuffer()
    //{
    //    return m_cbChangeOnCameraMovement;
    //}

    void Camera::HandleInput(_In_ const KeyboardInput& input, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_MoveBackForward += m_TravelSpeed * deltaTime * (input.IsButtonPressed(VK_UP) - input.IsButtonPressed(VK_DOWN));

        m_MoveLeftRight += m_TravelSpeed * deltaTime * (input.IsButtonPressed(VK_RIGHT)  - input.IsButtonPressed(VK_LEFT));

        m_MoveUpDown += m_TravelSpeed * deltaTime * (input.IsButtonPressed(VK_SPACE) - input.IsButtonPressed(VK_LSHIFT));

        if (mouseRelativeMovement.X != 0 || mouseRelativeMovement.Y != 0)
        {
            m_Yaw += static_cast<FLOAT>(mouseRelativeMovement.X) * m_RotationSpeed;
            if (m_Pitch + static_cast<FLOAT>(mouseRelativeMovement.Y) * m_RotationSpeed > -XM_PIDIV2 && m_Pitch + static_cast<FLOAT>(mouseRelativeMovement.Y) * m_RotationSpeed < XM_PIDIV2)
            {
                m_Pitch += static_cast<FLOAT>(mouseRelativeMovement.Y) * m_RotationSpeed;
            }
        }
    }

    //HRESULT Camera::Initialize(_In_  ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    //{
    //    UNREFERENCED_PARAMETER(pImmediateContext);
    //    HRESULT hr = S_OK;

    //    // Create the constant buffers
    //    D3D11_BUFFER_DESC bd = {};
    //    bd.Usage = D3D11_USAGE_DEFAULT;
    //    bd.ByteWidth = sizeof(CBChangeOnCameraMovement);
    //    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //    bd.CPUAccessFlags = 0;
    //    hr = pDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnCameraMovement.GetAddressOf());
    //    if (FAILED(hr))
    //    {
    //        return hr;
    //    }

    //    return hr;
    //}

    void Camera::Update(_In_ FLOAT deltaTime)
    {
        UNREFERENCED_PARAMETER(deltaTime);

        m_Rotation = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);

        m_At = XMVector3TransformCoord(DEFAULT_FORWARD, m_Rotation);
        m_At = XMVector3Normalize(m_At);

        XMMATRIX rotateYTempMatrix = XMMatrixRotationY(m_Yaw);

        m_CameraRight = XMVector3TransformCoord(DEFAULT_RIGHT, rotateYTempMatrix);
        m_CameraUp = XMVector3TransformCoord(DEFAULT_UP, rotateYTempMatrix);
        m_CameraForward = XMVector3TransformCoord(DEFAULT_FORWARD, rotateYTempMatrix);

        m_Eye += m_MoveLeftRight * m_CameraRight;
        m_Eye += m_MoveBackForward * m_CameraForward;
        m_Eye += m_MoveUpDown * m_CameraUp;

        m_MoveLeftRight = 0.0f;
        m_MoveBackForward = 0.0f;
        m_MoveUpDown = 0.0f;

        m_At = m_Eye + m_At;
        m_Up = m_CameraUp;

        m_View = XMMatrixLookAtLH(m_Eye, m_At, m_Up);
    }
}