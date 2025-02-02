#include "Player.h"
#include "pch.h"

#include <World/Physics/ColliderRegister.h>

Player::Player(WindowToRender& window, ColliderRegister& colliderRegister)
    : mCamera(window)
    , mCrosshairTexture("resources/Textures/crosshair.png")
    , mCrosshair(mCrosshairTexture)
    , mRifle(mCamera, colliderRegister)
    , mColliderRegister(colliderRegister)
    , mCollider(colliderRegister, {0, 0, 0}, {0.35, 0.8, 0.35})
{
    mCrosshair.setPosition({window.getSize().x / 2.f, window.getSize().y / 2.f},
                           Sprite2D::Origin::Center);
    mCrosshair.setOpacity(0.8f);
    mCamera.cameraPosition({mPosition.x, mPosition.y + PLAYER_HEGIHT, mPosition.z});
    mSoundBuffer.loadFromFile("resources/Sounds/footsteps.wav");
    mWalkingSound.setBuffer(mSoundBuffer);
    mWalkingSound.setVolume(50);
    mRifle.update(1);
}

void Player::draw(const Renderer& target) const
{
    mRifle.draw(target);
    mCrosshair.draw(target);
    mCollider.draw(target, mCamera);
}

void Player::update(const float& deltaTime)
{
    mCamera.cameraPosition({mPosition.x, mPosition.y + PLAYER_HEGIHT, mPosition.z});
    mCamera.update(deltaTime);
    mRifle.update(deltaTime);
}

void Player::updateColliderPosition()
{
    auto colliderPosition = mPosition - (mCollider.dimensions() / 2.f);
    colliderPosition.y = mPosition.y;
    mCollider.setPosition(colliderPosition);
}

bool Player::tryUpdatePositionByApplyingVelocityIfCollisionAllows(float& position, float& velocity)
{
    position += velocity;
    updateColliderPosition();
    auto collisions = mColliderRegister.findCollisions(mCollider.collider());
    std::erase_if(collisions,
                  [](const auto& collision)
                  {
                      return collision->colliderTag() != ColliderTag::Solid;
                  });
    if (not collisions.empty())
    {
        position -= velocity;
        velocity = 0;
        updateColliderPosition();
        return false;
    }
    return true;
}

void Player::updatePhysics(float deltaTime)
{
    handleMovementKeyboardInputs(deltaTime);
    decelerateVelocity(deltaTime);
    manageVerticalVelocity(deltaTime);
    limitVelocity(deltaTime);

    tryUpdatePositionByApplyingVelocityIfCollisionAllows(mPosition.x, mVelocity.x);
    doesPlayerStandOnCollider =
        not tryUpdatePositionByApplyingVelocityIfCollisionAllows(mPosition.y, mVelocity.y);
    if (mPosition.y < 0)
    {
        mPosition.y = 0;
    }
    tryUpdatePositionByApplyingVelocityIfCollisionAllows(mPosition.z, mVelocity.z);
}

void Player::decelerateVelocity(const float& deltaTime)
{
    float decayFactor = exp(-PLAYER_WALKING_DECELERATE_RATIO * deltaTime);
    mVelocity.x *= decayFactor;
    mVelocity.z *= decayFactor;
}

void Player::manageVerticalVelocity(const float& deltaTime)
{
    constexpr auto G = -9.81f * 0.01f;
    mVelocity.y += G * deltaTime;
}

void Player::limitVelocity(const float& deltaTime)
{
    static auto limitVelocitySpeed = [](auto& velocity, const auto& maxVelocity)
    {
        if (velocity > maxVelocity)
        {
            velocity = maxVelocity;
        }
        else if (-velocity > maxVelocity)
        {
            velocity = -maxVelocity;
        }
    };

    auto playerVelocity = PLAYER_MAX_HORIZONTAL_SPEED * deltaTime;
    limitVelocitySpeed(mVelocity.x, playerVelocity);
    limitVelocitySpeed(mVelocity.z, playerVelocity);


    if (-mVelocity.y > PLAYER_MAX_FALLING_SPEED)
    {
        mVelocity.y = -PLAYER_MAX_FALLING_SPEED;
    }
}

void Player::fixedUpdate(const float& deltaTime)
{
    updateColliderPosition();
    updatePhysics(deltaTime);
}

void Player::manageWalkingSounds(const glm::vec3& playerWalkingVector)
{
    constexpr auto noDirection = glm::vec3(0.0f, 0.0f, 0.0f);
    switch (mWalkingSound.getStatus())
    {
        case sf::Sound::Status::Playing:
            if (playerWalkingVector == noDirection or not isOnGround())
            {
                mWalkingSound.stop();
            }
            break;
        case sf::Sound::Status::Paused:
        case sf::Sound::Status::Stopped:
            if (playerWalkingVector != noDirection and isOnGround())
            {
                mWalkingSound.play();
            }
            break;
    }
}

void Player::handleMovementKeyboardInputs(const float& deltaTime)
{
    auto ACCELERATION_RATIO = 0.1f;
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        direction += mCamera.directionWithoutPitch();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        direction -= mCamera.directionWithoutPitch();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        direction += mCamera.rightDirectionWithoutPitch();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        direction -= mCamera.rightDirectionWithoutPitch();
    }
    manageWalkingSounds(direction);

    if (glm::length(direction) > 0.0f)
    {
        direction = glm::normalize(direction);// Normalize the combined direction
        const auto finalSpeed = PLAYER_WALKING_SPEED * ACCELERATION_RATIO * deltaTime;
        mVelocity += finalSpeed * direction;
    }
}

void Player::tryJump()
{
    if (isOnGround())
    {
        mVelocity.y = PLAYER_JUMP_FORCE;
    }
}

void Player::handleEvent(const sf::Event& event)
{
    switch (event.key.code)
    {
        case sf::Keyboard::Space: tryJump(); break;
    }
    mRifle.handleEvent(event);
}

Camera& Player::camera()
{
    return mCamera;
}

const Camera& Player::camera() const
{
    return mCamera;
}

bool Player::isOnGround() const
{
    return mPosition.y <= 0 or doesPlayerStandOnCollider;
}
