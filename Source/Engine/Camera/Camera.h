/*+===================================================================
  File:     CAMERA.H

  Summary:  Camera header file contains declarations of Camera class

  Classes:  Camera

  ¨Ï 2022    Minha Ju
===================================================================+*/

#pragma once

//#include "Graphics/DataTypes.h"
#include "Input/Input.h"

namespace pr
{
    /*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
        Class:    Camera
        Summary:  Immitates a camera that moves according to the WASD and
                mouse input.
        Methods:  GetEye
                    Getter for the eye vector
                GetAt
                    Getter for the at vector
                GetUp
                    Getter for the up vector
                GetView
                    Getter for the view transform matrix
                HandleInput
                    Handles the keyboard / mouse input
                Initialize
                    Initialize the data Camera uses
                Update
                    Update the camera according to the input
                Camera
                    Constructor.
                ~Camera
                    Destructor.
    C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/
    class Camera
    {
    public:
        struct ConstantBuffer
        {
            XMMATRIX View;
            XMFLOAT4 Position;
        };

    public:
        Camera() = delete;
        Camera(_In_ const XMVECTOR& position);
        Camera(const Camera& other) = delete;
        Camera(Camera&& other) = delete;
        Camera& operator=(const Camera& other) = delete;
        Camera& operator=(Camera&& other) = delete;
        virtual ~Camera() = default;

        const XMVECTOR& GetEye() const;
        const XMVECTOR& GetAt() const;
        const XMVECTOR& GetUp() const;
        const XMMATRIX& GetView() const noexcept;
        //ConstantBuffer GetConstantBuffer() const noexcept;

        virtual void HandleInput(_In_ const KeyboardInput& Input, _In_ const MouseInput& MouseRelativeMovement, _In_ FLOAT deltaTime);
        //virtual HRESULT Initialize(_In_ ID3D11Device* device, _In_ ID3D11DeviceContext* pImmediateContext);
        virtual void Update(_In_ FLOAT deltaTime);
    protected:
        static constexpr const XMVECTORF32 DEFAULT_FORWARD = { 0.0f, 0.0f, 1.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_RIGHT = { 1.0f, 0.0f, 0.0f, 0.0f };
        static constexpr const XMVECTORF32 DEFAULT_UP = { 0.0f, 1.0f, 0.0f, 0.0f };
        static constexpr const FLOAT MAX_TRAVEL_SPEED = 128.0f;
        static constexpr const FLOAT DEFAULT_TRAVEL_SPEED = 12.0f;
        static constexpr const FLOAT MIN_TRAVEL_SPEED = 1.0f;
        //static constexpr const FLOAT MAX_ROTATION_SPEED = 1.0f;
        static constexpr const FLOAT DEFAULT_ROTATION_SPEED = 0.5f;
        //static constexpr const FLOAT MIN_ROTATION_SPEED = 0.0004f;

    //    ComPtr<ID3D11Buffer> m_cbChangeOnCameraMovement;

        XMMATRIX m_Rotation;            // 80 bytes
        XMMATRIX m_View;                // 144 bytes

        XMVECTOR m_CameraForward;       // 160 bytes
        XMVECTOR m_CameraRight;         // 176 bytes
        XMVECTOR m_CameraUp;            // 192 bytes

        XMVECTOR m_Eye;                 // 208 bytes
        XMVECTOR m_At;                  // 224 bytes
        XMVECTOR m_Up;                  // 240 bytes

        FLOAT m_Yaw;                    // 240 bytes
        FLOAT m_Pitch;                  // 240 bytes

        FLOAT m_MoveLeftRight;          // 256 bytes
        FLOAT m_MoveBackForward;        // 256 bytes
        FLOAT m_MoveUpDown;             // 256 bytes

        FLOAT m_TravelSpeed;            // 256 bytes
        FLOAT m_RotationSpeed;          // 272 bytes
    };
    static_assert(sizeof(Camera) % 16 == 0);
    static_assert(sizeof(Camera) == 272);
}