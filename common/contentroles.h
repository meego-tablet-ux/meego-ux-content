#ifndef __contentroles_h
#define __contentroles_h

class McaContentRoles
{
public:
    enum Roles {
        SystemUpidRole = Qt::UserRole + 1,
        SystemEnabledRole = Qt::UserRole + 2
    };

    enum ServiceRoles {
        SystemServiceIdRole =       Qt::UserRole + 1,  // QString
        SystemServiceNameRole =     Qt::UserRole + 2,  // QString
        SystemServiceIconRole =     Qt::UserRole + 3,  // QString
        SystemServiceCategoryRole = Qt::UserRole + 4,  // QString
    };

private:
    McaContentRoles() {}
};


#endif //__contentroles_h
