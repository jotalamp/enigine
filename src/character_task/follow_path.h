#ifndef follow_path_hpp
#define follow_path_hpp

#include <iostream>
#include <string>

#include "character_task.h"
class Character;
class NPCharacter;
class PCharacter;
#include "../character/character.h"

class FollowPath : public CharacterTask
{
public:
    FollowPath(Character *source, std::vector<glm::vec3> path, glm::vec3 lookDir);
    ~FollowPath();

    Character *m_source;
    std::vector<glm::vec3> m_path;
    glm::vec3 m_lookDir;

    bool m_positionReady = false;
    bool m_orientationReady = false;

    void interrupt();
    bool update();
    bool follow(glm::vec3 point);
};

#endif /* follow_path_hpp */
