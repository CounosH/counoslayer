#ifndef COUNOSH_QT_COUNOSCORE_INIT_H
#define COUNOSH_QT_COUNOSCORE_INIT_H

namespace CounosCore
{
    //! Shows an user dialog with general warnings and potential risks
    bool AskUserToAcknowledgeRisks();

    //! Setup and initialization related to Counos Core Qt
    bool Initialize();
}

#endif // COUNOSH_QT_COUNOSCORE_INIT_H
