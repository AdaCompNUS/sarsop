#ifndef Bound_H
#define Bound_H

#include <vector>
#include "Const.h"
#include "MathLib.h"
#include "Belief.h"
#include "PointBasedAlgorithm.h"

using namespace std;
using namespace momdp;
namespace momdp 
{
	class MObject;
	class BeliefTreeNode;
	class BeliefCache;

	template <typename T> class Backup;

	template <typename T>
	class Bound
	{
	public:
		typedef void (*BackupCallback)(PointBasedAlgorithm *solver, BeliefTreeNode * node, SharedPointer<T> backupResult) ;
		Backup<T> *backupEngine;

	protected:
		vector<BackupCallback> onBackup;
		PointBasedAlgorithm *solver;

		void doNothing(BeliefTreeNode * node, SharedPointer<T> backupResult)
		{
		}

	public:

		//Bound(Backup<T> *_backupEngine)
		//{
		//	backupEngine = _backupEngine;
		//	onBackup = &doNothing;
		//}

		virtual void appendOnBackupHandler(BackupCallback _onBackup)
		{
			onBackup.push_back( _onBackup);
		}
		virtual void removeOnBackupHandler(BackupCallback _onBackup)
		{
			typename vector<BackupCallback>::iterator iter;
			for(iter = onBackup.begin(); iter!= onBackup.end() ; iter ++)
			{
				if(_onBackup == *iter)
				{
					onBackup.erase( iter);
					break;
				}
			}
		}

		void setBackupEngine(Backup<T> *_backupEngine)
		{
			backupEngine = _backupEngine;
		}

		virtual ~Bound(void)
		{
		}

		//virtual REAL_VALUE getValue(SharedPointer<Belief>& belief) = 0;
		// Different type of Beliefs have no common interface, therefore, should not explicitly use it

		virtual SharedPointer<T> backup(BeliefTreeNode * node) = 0;

	};

}


#endif

