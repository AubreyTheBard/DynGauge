/*! \file DynGauge.cpp

    \brief DynRPG plugin to display health, mana, and ATB gauges, plus status condition icons and counters, for Battlers

    \author Bernard J. Badger, AKA Aubrey the Bard

    DynGauge is a DynRPG plugin which allows the display of gauges above both heroes and monsters,
    plus icons and counters for status conditions. It allows for control over which parts of the
    display are shown over heroes or monsters, when they are shown, and details of how they are
    shown.
*/


#include <DynRPG/DynRPG.h>
#define NOT_MAIN_MODULE

//! Battle display for a single Battler
/*!
    This class handles the modeling and displaying of battle information for a single Battler (hero
    or monster) including gauges for health, mana, and ATB, plus
*/
class BattleDisplay
{

public:

    const static int DISPLAY_WIDTH = 80;                //!< Width of the display Image (TODO: probably will change this, currently set to more than it needs to be for experimental purposes)
    const static int DISPLAY_HEIGHT = 320;              //!< Height of the display Image (TODO: probably will change this, currently set to more than it needs to be for experimental purposes)
    const static int IMAGE_UNIT_SIZE = 8;               //!< Unit size for Images (smallest possible width/height for an Image)
    const static int GAUGE_WIDTH = 40;                  //!< Width of a gauge Image
    const static int GAUGE_HEIGHT = 8;                  //!< Height of a gauge Image
    const static int HEALTH_GAUGE_SRC_X = 0;            //!< Source X coordinate of health gauge in SystemGraphic
    const static int HEALTH_GAUGE_SRC_Y = 40;           //!< Source Y coordinate of health gauge in SystemGraphic
    const static int MANA_GAUGE_SRC_X = 0;              //!< Source X coordinate of mana gauge in SystemGraphic
    const static int MANA_GAUGE_SRC_Y = 56;             //!< Source Y coordinate of mana gauge in SystemGraphic
    const static int ATB_GAUGE_SRC_X = 0;               //!< Source X coordinate of ATB gauge in SystemGraphic
    const static int ATB_GAUGE_SRC_Y = 72;              //!< Source Y coordinate of ATB gauge in SystemGraphic
    const static int BAR_WIDTH = 40;                    //!< Width of a bar Image
    const static int BAR_HEIGHT = 8;                    //!< Height of a bar Image
    const static int HEALTH_BAR_A_SRC_X = 48;           //!< Source X coordinate of health bar A in SystemGraphic
    const static int HEALTH_BAR_A_SRC_Y = 40;           //!< Source Y coordinate of health bar A in SystemGraphic
    const static int MANA_BAR_A_SRC_X = 48;             //!< Source X coordinate of mana bar A in SystemGraphic
    const static int MANA_BAR_A_SRC_Y = 56;             //!< Source Y coordinate of mana bar A in SystemGraphic
    const static int ATB_BAR_A_SRC_X = 48;              //!< Source X coordinate of ATB bar A in SystemGraphic
    const static int ATB_BAR_A_SRC_Y = 72;              //!< Source Y coordinate of ATB bar A in SystemGraphic
    const static int HEALTH_BAR_B_SRC_X = 64;           //!< Source X coordinate of health bar B in SystemGraphic
    const static int HEALTH_BAR_B_SRC_Y = 40;           //!< Source Y coordinate of health bar B in SystemGraphic
    const static int MANA_BAR_B_SRC_X = 64;             //!< Source X coordinate of mana bar B in SystemGraphic
    const static int MANA_BAR_B_SRC_Y = 56;             //!< Source Y coordinate of mana bar B in SystemGraphic
    const static int ATB_BAR_B_SRC_X = 64;              //!< Source X coordinate of ATB bar B in SystemGraphic
    const static int ATB_BAR_B_SRC_Y = 72;              //!< Source Y coordinate of ATB bar B in SystemGraphic
    const static int DIGIT_WIDTH = 8;                   //!< Width of a digit Image
    const static int DIGIT_HEIGHT = 16;                 //!< Height of a digit Image
    const static int DIGIT_SRC_X = 0;                   //!< Source X coordinate of the first digit in SystemGraphic
    const static int DIGIT_SRC_Y = 80;                  //!< Source Y coordinate of the first digit in SystemGraphic
    const static int NUM_DIGITS = 10;                   //!< Amount of digit Images

    static RPG::Image * mHealthGaugePtr;                //!< Pointer to an Image of the health gauge
    static RPG::Image * mManaGaugePtr;                  //!< Pointer to an Image of the mana gauge
    static RPG::Image * mATBGaugePtr;                   //!< Pointer to an Image of the ATB gauge
    static RPG::Image * mHealthBarAPtr;                 //!< Pointer to an Image of health bar A ("non-full")
    static RPG::Image * mHealthBarBPtr;                 //!< Pointer to an Image of health bar B ("full")
    static RPG::Image * mManaBarAPtr;                   //!< Pointer to an Image of mana bar A ("non-full")
    static RPG::Image * mManaBarBPtr;                   //!< Pointer to an Image of mana bar B ("full")
    static RPG::Image * mATBBarAPtr;                    //!< Pointer to an Image of ATB bar A ("non-full")
    static RPG::Image * mATBBarBPtr;                    //!< Pointer to an Image of ATB bar B ("full")
    static RPG::Image * mDigitPtr[NUM_DIGITS];          //!< Array of pointers to Image of numerical digits 0-9

    //! Default constructor
    /*!
        The default constructor of BattleDisplay provides an empty instance.
    */
    BattleDisplay()
    {

        // Initialize variables
        mBattlerPtr = NULL;
        mCurHealth = 0;
        mCurMana = 0;
        mCurATB = 0;
        mDisplayPtr = RPG::Image::create( DISPLAY_WIDTH, DISPLAY_HEIGHT );

    }

    //! Constructor
    /*!
        This constructor creates a BattleDisplay for a given Battler.

        \param rBattlerPtr : (RPG::Battler *) Pointer to the Battler which this BattleDisplay will serve
    */
    BattleDisplay( RPG::Battler * rBattlerPtr )
    {

        // Initialize variables
        mBattlerPtr = rBattlerPtr;
        mCurHealth = mBattlerPtr->hp;
        mCurMana = mBattlerPtr->mp;
        mCurATB = mBattlerPtr->atbValue;
        mDisplayPtr = RPG::Image::create( DISPLAY_WIDTH, DISPLAY_HEIGHT );

    }

    //! Destructor
    /*!
        The destructor of BattleDisplay deletes all member objects and the BattleDisplay object
        itself.
    */
    ~BattleDisplay()
    {

        // Destroy the image used to display battle info
        RPG::Image::destroy( mDisplayPtr );
        // Note that since mBattlerPtr merely points to a Battler object which exists outside of
        // the context of this plugin, it does not have to (nor should it) be deleted/destroyed.

    }

    //! Sets the Battler
    /*!
        SetBattler() sets the value of mBattlerPtr to the given parameter and initializes related
        variables according to the Battler's data.

        \param rBattlerPtr : (RPG::Battler *) Pointer to the Battler which this BattleDisplay will serve
    */
    void SetBattler( RPG::Battler * rBattlerPtr )
    {

        // If necessary, initialize static members
        if( !mInitialized )
        {

            InitializeStatic();

        }
        // Initialize variables
        mBattlerPtr = rBattlerPtr;
        mCurHealth = rBattlerPtr->hp;
        mCurMana = mBattlerPtr->mp;
        mCurATB = mBattlerPtr->atbValue;

    }

    //! Updates the BattleDisplay
    /*!
        Update() recalculates values based on past and present data and calls Draw() to refresh the
        appearance of the BattleDisplay.
    */
    void Update()
    {

        // Update variables
        mCurHealth = mBattlerPtr->hp;
        mCurMana = mBattlerPtr->mp;
        mCurATB = mBattlerPtr->atbValue;
        // Refresh display
        Draw();

    }

private:

    static bool mInitialized;                           //!< Flag indicating whether the static members of the class have been initialized

    int mCurHealth;                                     //!< Current health
    int mCurMana;                                       //!< Current mana
    int mCurATB;                                        //!< Current ATB fill value

    RPG::Image * mDisplayPtr;                           //!< Pointer to display Image
    RPG::Battler * mBattlerPtr;                         //!< Pointer to Battler for which this BattleDisplay is used

    //! Initializes static member variables
    /*!
        This method initializes the static member variables of the class. It should be called
        before any instances of the class are created.
    */
    static void InitializeStatic()
    {

        int i;          // Index variable

        // Initialize images
        mHealthGaugePtr->draw( 0, 0,                                            // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               HEALTH_GAUGE_SRC_X, HEALTH_GAUGE_SRC_Y,          // Coordinates in source Image
                               GAUGE_WIDTH, GAUGE_HEIGHT,                       // Dimensions in source Image
                               0);                                              // Transparency color
        mManaGaugePtr->draw( 0, 0,                                              // Coordinates in destination Image
                             RPG::system->systemGraphic->system2Image,          // Source Image pointer
                             MANA_GAUGE_SRC_X, MANA_GAUGE_SRC_Y,                // Coordinates in source Image
                             GAUGE_WIDTH, GAUGE_HEIGHT,                         // Dimensions in source Image
                             0);                                                // Transparency color
        mATBGaugePtr->draw( 0, 0,                                               // Coordinates in destination Image
                            RPG::system->systemGraphic->system2Image,           // Source Image pointer
                            ATB_GAUGE_SRC_X, ATB_GAUGE_SRC_Y,                   // Coordinates in source Image
                            GAUGE_WIDTH, GAUGE_HEIGHT,                          // Dimensions in source Image
                            0);                                                 // Transparency color
        mHealthBarAPtr->draw( 0, 0,                                             // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               HEALTH_BAR_A_SRC_X, HEALTH_BAR_A_SRC_Y,          // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        mManaBarAPtr->draw( 0, 0,                                               // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               MANA_BAR_A_SRC_X, MANA_BAR_A_SRC_Y,              // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        mATBBarAPtr->draw( 0, 0,                                                // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               ATB_BAR_A_SRC_X, ATB_BAR_A_SRC_Y,                // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        mHealthBarBPtr->draw( 0, 0,                                             // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               HEALTH_BAR_B_SRC_X, HEALTH_BAR_B_SRC_Y,          // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        mManaBarBPtr->draw( 0, 0,                                               // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               MANA_BAR_B_SRC_X, MANA_BAR_B_SRC_Y,              // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        mATBBarBPtr->draw( 0, 0,                                                // Coordinates in destination Image
                               RPG::system->systemGraphic->system2Image,        // Source Image pointer
                               ATB_BAR_B_SRC_X, ATB_BAR_B_SRC_Y,                // Coordinates in source Image
                               BAR_WIDTH, BAR_HEIGHT,                           // Dimensions in source Image
                               0);                                              // Transparency color
        for( i = 0; i < NUM_DIGITS; i++ )
        {

            mDigitPtr[i]->draw( 0, 0,                                           // Coordinates in destination Image
                                   RPG::system->systemGraphic->system2Image,    // Source Image pointer
                                   DIGIT_SRC_X + DIGIT_WIDTH * i, DIGIT_SRC_Y,  // Coordinates in source Image
                                   DIGIT_WIDTH, DIGIT_HEIGHT,                   // Dimensions in source Image
                                   0);                                          // Transparency color

        }
        // Turn on initialized flag
        mInitialized = true;

    }

    //! Draws the display image
    /*!
        This method draws the display Image based on the relevant data and display rules and puts
        it on the Canvas.
    */
    void Draw()
    {

        static int curX, curY;                  // Current coordinates within the display Image

        // Clear the display Image
        mDisplayPtr->clear();
        // Find starting position: lower-left corner of display Image with enough room for one gauge
        curX = 0;
        curY = DISPLAY_HEIGHT - GAUGE_HEIGHT;
        // Draw the health gauge
        mDisplayPtr->draw( curX, curY,                                          // Coordinates in destination Image
                           mHealthGaugePtr,                                     // Source Image pointer
                           0, 0,                                                // Coordinates in source Image
                           GAUGE_WIDTH, GAUGE_HEIGHT,                           // Dimensions in source Image
                           0);

    }

};

bool BattleDisplay::mInitialized = false;
RPG::Image * BattleDisplay::mHealthGaugePtr = RPG::Image::create( BattleDisplay::GAUGE_WIDTH, BattleDisplay::GAUGE_HEIGHT );
RPG::Image * BattleDisplay::mManaGaugePtr = RPG::Image::create( BattleDisplay::GAUGE_WIDTH, BattleDisplay::GAUGE_HEIGHT );
RPG::Image * BattleDisplay::mATBGaugePtr = RPG::Image::create( BattleDisplay::GAUGE_WIDTH, BattleDisplay::GAUGE_HEIGHT );
RPG::Image * BattleDisplay::mHealthBarAPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mManaBarAPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mATBBarAPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mHealthBarBPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mManaBarBPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mATBBarBPtr = RPG::Image::create( BattleDisplay::BAR_WIDTH, BattleDisplay::BAR_HEIGHT );
RPG::Image * BattleDisplay::mDigitPtr[BattleDisplay::NUM_DIGITS] = {
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ),
    RPG::Image::create( BattleDisplay::DIGIT_WIDTH, BattleDisplay::DIGIT_HEIGHT ) };

const int NUM_HEROES = 4;                               //!< Maximum number of heroes
const int NUM_MONSTERS = 8;                             //!< Maximum number of monsters

std::map<std::string, std::string> configuration;       //!< Configuration data from the DynRPG.ini file


bool inBattle;                                          //!< Whether the game is currently in a battle

BattleDisplay heroBattleDisplay[NUM_HEROES];            //!< Battle displays for heroes
BattleDisplay monsterBattleDisplay[NUM_MONSTERS];       //!< Battle displays for monsters

bool onStartup( char *pluginName )
{

    int i;          // Index variable

    // Initialize variables
    inBattle = false;
	configuration = RPG::loadConfiguration( pluginName );

	return true;

}

//! Called every frame
/*!
    onFrame() is called on every frame of the game loop. In this plugin this method is used to
    detect and react to transitions between battles and other scenes.

    \param scene : ( RPG::Scene ) Enumerator for what type of scene the game is currently in
*/
void onFrame( RPG::Scene scene )
{

    static int i;           // Index variable

    if( inBattle )
    {   // Game was in a battle scene at last check

        if( RPG::SCENE_BATTLE != scene )
        {   // Current scene is not a battle; battle just ended!

            inBattle = false;

        }

    }
    else
    {   // Game was not in a battle scene at last check

        if( RPG::SCENE_BATTLE == scene )
        {   // Current scene is a battle; battle just started!

            inBattle = true;
            // Assign BattleDisplays for all active Battlers
            for( i = 0; i < NUM_MONSTERS; i++ )
            {

                if( 0 != RPG::monsters[i]->databaseId )
                {

                    monsterBattleDisplay[i].SetBattler( RPG::monsters[i] );

                }

            }

        }

    }

}

//! Called immediately after a Battler is drawn
/*!
    onBattlerDrawn() is called immediately after a Battler is drawn to the Canvas. In this plugin
    this method is used to update the BattleDisplays.

    \param battler : ( RPG::Battler * ) The battler which was drawn (or supposed to be drawn)
    \param isMonster: ( bool ) true if the battler is a monster
    \param id: ( int ) Zero-based party member ID of the battler
*/
//bool onBattlerDrawn( RPG::Battler *battler, bool isMonster, int id )
//{
//
//    // Call Update on appropriate BattleDisplay
//    if( isMonster )
//    {
//
//        monsterBattleDisplay[id].Update();
//
//    }
//    else
//    {
//
//        heroBattleDisplay[id].Update();
//
//    }
//    return true;
//
//}

//! Clean up after use
/*!
    onExit() is called when the game closes. In this plugin this is used to perform any needed
    cleanup.
*/
void onExit()
{

    int i;          // Index variable

    // Destroy static Images of BattleDisplay class
    RPG::Image::destroy( BattleDisplay::mHealthGaugePtr );
    RPG::Image::destroy( BattleDisplay::mManaGaugePtr );
    RPG::Image::destroy( BattleDisplay::mATBGaugePtr );
    RPG::Image::destroy( BattleDisplay::mHealthBarAPtr );
    RPG::Image::destroy( BattleDisplay::mHealthBarBPtr );
    RPG::Image::destroy( BattleDisplay::mManaBarAPtr );
    RPG::Image::destroy( BattleDisplay::mManaBarBPtr );
    RPG::Image::destroy( BattleDisplay::mATBBarAPtr );
    RPG::Image::destroy( BattleDisplay::mATBBarBPtr );
    for( i = 0; i < BattleDisplay::NUM_DIGITS; i++ )
    {

        RPG::Image::destroy( BattleDisplay::mDigitPtr[i] );

    }

}
