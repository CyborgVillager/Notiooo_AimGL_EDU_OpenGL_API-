#pragma once


/**
 * By default, the game does not need the opengl graphics module. At this point it is used only
 * debug for the purpose of drawing imgui. Therefore, a different window is used in Debug mode than
 * in Release mode. This will be changed later.
 **/
#ifdef _DEBUG
    #include <SFML/Graphics/RenderWindow.hpp>
using WindowToRender = sf::RenderWindow;
#else
    #include <SFML/Window.hpp>
using WindowToRender = sf::Window;
#endif


#include "States/StateStack.h"

/**
 * \brief The main game class that controls the entire flow of the application.
 *
 * The whole task of this class is the run() function,
 * which ensures that the game runs. It runs the processes
 * of displaying the game (image), capturing player input
 * and updating the game logic.
 */
class Game
{
public:
    Game();
    /**
     * \brief Starts the game and keeps it running until the player finishes it.
     *
     * Updates the game logic, displays the game image and captures the
     * player inputs to the various parts of the program.
     */
    void run();

private:
    /**
     * \brief The main loop that controls the operation of the game in the loop.
     *
     * Updates the game logic, displays the game image and captures the
     * player inputs to the various parts of the program.
     */
    void performGameLoop();

    /**
     * \brief Intercepts player inputs and passes them to processes inside the game.
     */
    void processEvents();

    /**
     * \brief Updates the game logic at equal intervals independent of the frame rate.
     * \param deltaTime Time interval
     *
     * This function is very aggravating and highly inefficient. In a certain amount of time it
     * executes an equal number of times by which it is ideal, for example, for collision
     * recalculation. Thus, in case of lag, there are no situations where the player can run through
     * a wall. Intermediate intervals regardless of the frame rate are always recalculated!
     *
     * Updates the game logic by passing the time that has elapsed
     * since the previous interval. This allows objects to move independently
     * of the speed at which subsequent iterations of the program are executed.
     * (distance = speed * time)
     */
    void fixedUpdate(const sf::Time& deltaTime);

    /**
     * It performs fixed updates at equal intervals. In case of huge time gaps, it makes up for it
     * by executing one by one successive calls with the same fixed time argument. to avoid behavior
     * where, due to high lag, a character is moved off the wall avoiding collision checking.
     */
    void fixedUpdateAtEqualIntervals();

    /**
     * \brief Updates ImGui window displaying start and stop panel for trace collection
     */
    void updateImGuiMiniTrace();

    /**
     * \brief Creates ImGui objects (every frame)
     * \param deltaTime Time elapsed since the previous frame
     */
    void updateImGui(sf::Time deltaTime);

    /**
     * \brief Updates the game logic dependent, or independent of time, every rendered frame.
     * \param deltaTime the time that has passed since the game was last updated.
     *
     * It is not recommended to use this feature for physics, or general movement.
     * Updates the game logic by passing the time that has elapsed since the previous
     * update call. This allows objects to move independently of the speed at which
     * subsequent frames of the program are executed.
     * (distance = speed * time)
     */
    void update(const sf::Time& deltaTime);

    /**
     * \brief Displays the game on the image of the game window
     *
     * It collects all the game elements that you want to display in the current
     * iteration and displays them on the screen in the order they appear in the code.
     * It clears the screen with a black image before displaying a new frame.
     */
    void render();

    /**
     * \brief Loads the resources needed by the processes inside the game.
     *
     * Not all available resources are listed here, as some are loaded on the fly
     * inside the game. This is because for example there is no need to keep the whole
     * game in memory when the player is really only inside the game's main menu.
     */
    void loadResources();

    /**
     * @brief The time it takes for one game frame to be generated.
     */
    static const sf::Time TIME_PER_FRAME;

    /**
     * @brief Time between one fixed update and another
     */
    static const sf::Time TIME_PER_FIXED_UPDATE_CALLS;

    /**
     * @brief Default game window width
     */
    static const int SCREEN_WIDTH;

    /**
     * @brief Default game window height
     */
    static const int SCREEN_HEIGHT;

    /**
     * @brief The window to which the game image should be drawn.
     */
    std::unique_ptr<WindowToRender> mGameWindow;

    bool isGameRunning = true;

    /**
     * @brief A clock used to determine the last time the fixedUpdate function was called
     */
    sf::Clock mFixedUpdateClock;

    /**
     * @brief Time since the last call to the fixedUpdate function
     */
    sf::Time mTimeSinceLastFixedUpdate = sf::Time::Zero;

    /**
     * \brief Stores and manages in-game states.
     *
     * This allows you to create some application flow between application states.
     * Among other things, it allows to go from the main menu of the game to the game
     * itself, as well as to pause the game.
     */
    StateStack mAppStack;
};
