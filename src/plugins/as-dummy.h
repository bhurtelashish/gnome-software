

#ifndef __GS_ASDUMMY_H
#define __GS_ASDUMMY_H

#define AsIdKind AsComponentKind

/**
 * AsAppState:
 * @AS_APP_STATE_UNKNOWN:			Unknown state
 * @AS_APP_STATE_INSTALLED:			Application is installed
 * @AS_APP_STATE_AVAILABLE:			Application is available
 * @AS_APP_STATE_AVAILABLE_LOCAL:		Application is locally available as a file
 * @AS_APP_STATE_UPDATABLE:			Application is installed and updatable
 * @AS_APP_STATE_UNAVAILABLE:			Application is referenced, but not available
 * @AS_APP_STATE_QUEUED_FOR_INSTALL:		Application is queued for install
 * @AS_APP_STATE_INSTALLING:			Application is being installed
 * @AS_APP_STATE_REMOVING:			Application is being removed
 *
 * The application state.
 **/
typedef enum {
	AS_APP_STATE_UNKNOWN,				/* Since: 0.2.2 */
	AS_APP_STATE_INSTALLED,				/* Since: 0.2.2 */
	AS_APP_STATE_AVAILABLE,				/* Since: 0.2.2 */
	AS_APP_STATE_AVAILABLE_LOCAL,			/* Since: 0.2.2 */
	AS_APP_STATE_UPDATABLE,				/* Since: 0.2.2 */
	AS_APP_STATE_UNAVAILABLE,			/* Since: 0.2.2 */
	AS_APP_STATE_QUEUED_FOR_INSTALL,		/* Since: 0.2.2 */
	AS_APP_STATE_INSTALLING,			/* Since: 0.2.2 */
	AS_APP_STATE_REMOVING,				/* Since: 0.2.2 */
	AS_APP_STATE_LAST
} AsAppState;

#endif
