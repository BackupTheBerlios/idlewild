/*####################################################################################################
 project: IdleWild
 Author: nEuRoMaNcEr (Marcio Vinicius Mello de Freitas)
 module:  IW_SQL
 Description: This class is used at server side to provide abstraction on database conections
   and data manipulation such as: login-logout-register-passwd_recouver-passwd_change. Made to be simple

todo: work on compatibility stuff beetwn MYSQL, POSTGRE SQL AND SYBASE . It works fine with
      all of them but some tiny peaces of code must are DB dependent....
      
      the chpasswd() and lostpasswd() are not done.. coz MD5 encryption is going
      to be added soon to this class, this means that no crypt/decrypt calls will
      *NOT* be nescessary... the is going to implement it transparently
#####################################################################################################*/

// QT libs
#include <qstring.h>
#include <qsqlquery.h>
#include <qsqlrecord.h>
#include <qsqlcursor.h>
#include <qsqldatabase.h>
#include <qapplication.h>
// Standard libs
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Some constants
#define SQL 0 
#define PSQL 1 
#define SYBASE 2 

    
class IW_SQL
{

 public:

	void IW_setConnectionInfo ( const char* db_username, const char* db_password, const char* db_databasename ,short int db_vendor);
	void IW_showConnectionInfo(void);
	bool IW_DbConnect(const char* db_serverhost);
	unsigned int IW_register(const char* iw_user,const char* iw_passwd,const char* iw_currentip ,const char* iw_mail);
	bool IW_logIn(const char* iw_user, const char* iw_currentip);
	bool IW_logOut(const char* iw_user);
	//char* IW_getLastError();
	
 private:
 	virtual bool IW_isThere(const char* username);
	char *db_user,*db_passwd,*db_name;			// this is db_server account not IdleWild account
	char *vendor;						// Database driver to manage the conversation betewwn the server and us	 
    	char *db_last_err;
	
};

/*###################################################################################################
 IW_setConnectionInfo()
 			Recives the username,password and database driver name. The Database driver 
 name is an integer value, defined with the following names: SQL3=0, PSQL=1, SYBASE=2
 these are the free QT database drivers for mysql, PostgreSQL and Sybase Adaptive server respectivly
 * note that username and password is the DBSERVER account not a user accout to idlewild 
###################################################################################################*/

void IW_SQL::IW_setConnectionInfo ( const char* db_username, const char* db_password, const char* db_databasename ,short int db_vendor)
{
	db_user = (char*) db_username;
	db_passwd = (char*)db_password;
	db_name = (char*)db_databasename;
	
	switch (db_vendor)
	{
		case SQL:
			vendor = "QMYSQL3";
			break;
		case PSQL:
			vendor = "QPSQL7";
			break;
		case SYBASE:
			vendor = "QTDS7";
			break;
		default:
			vendor = "QMYSQL3";
			
	}
	

}
/*#####################################################################################################
IW_isThere(username) return true if the username is already present in db, and  false otherwise
#####################################################################################################*/
bool IW_SQL::IW_isThere(const char* username)
{
	QSqlQuery q1;
	QString s="SELECT username FROM iwusers WHERE username='";
	s.append(username).append("'");
	q1.exec(s);
	return (q1.next())?true:false;
}

/*#####################################################################################################
IW_LogIn(const char* iw_user, const char* iw_currentip)
			set status = 1 to user iw_user and stores his ip address
			and even stores when user logged in
			returns true if user exist and no erro occous, false otherwise
#####################################################################################################*/
bool IW_SQL::IW_logIn(const char* iw_user, const char* iw_currentip)
{
	// Is iw_user a valid user ?
	if ( IW_isThere(iw_user) )
	{	
		QString s="username='";	
		s.append(iw_user).append("'");
		QSqlCursor cur("iwusers");	
		cur.select(s);
		
		if (cur.next())
		{	QSqlRecord *buffer = cur.primeUpdate();
			qDebug(cur.value("username").toString());
			buffer->setValue("status","1");				// Change hist status
			buffer->setValue("ipaddr",iw_currentip);		// And updates his ip address
			QSqlQuery q1;
			q1.exec("SELECT NOW()");
			q1.next();
			buffer->setValue("since",q1.value(0).toString());	// log when user came in
			cur.update();						// just do it! 
		
			return true;
		}
		return false;	
	}
		return false;
}

/*#####################################################################################################
IW_LogOut(const char* iw_user)   What ? didn't u understand it ?
#####################################################################################################*/
bool IW_SQL::IW_logOut(const char* iw_user)
{
	QString s="username='";	
	s.append(iw_user).append("'");
	QSqlCursor cur("iwusers");	
	cur.select(s);
	
	if (cur.next())
	{	QSqlRecord *buffer = cur.primeUpdate();
		buffer->setValue("status","0");		// Change hist status
		QSqlQuery q1;
		cur.update();				// just do it! 	
		cur.next();
		return true;
	}	
	 return false;
}
		
void IW_SQL::IW_showConnectionInfo (void)
{
	printf( "login: %s\nPassword: %s\n Driver: %s\n Database: %s",db_user, db_passwd,vendor,db_name);
}

/*#####################################################################################################
IW_DbConnect(db_serverhost) connect to dbserver at host db_serverhost with account information
			    set by IW_setConnectionInfo()
######################################################################################################*/
bool IW_SQL::IW_DbConnect( const char* db_serverhost )
{
   bool retval;
   
   	QSqlDatabase *sqldb = QSqlDatabase::addDatabase( vendor );
	sqldb->setHostName( db_serverhost );	
	sqldb->setDatabaseName( db_name );
	sqldb->setPassword( db_passwd );
        sqldb->setUserName( db_user );
        printf ("connecting %s\n",db_serverhost);        
        if ( sqldb->open() ) 
	{         		  
		  retval = true;		  
        }
	else
	{
		retval = false;
	}
	
	return retval;

}

/*#####################################################################################################
 IW_register() 
 		Recives: Username,password,current ip address,e-mail address, returns 0 on error
		otherwise it returns the ID number of the new accout		
#####################################################################################################*/
unsigned int IW_SQL::IW_register(const char* iw_user,const char* iw_passwd,const char* iw_currentip ,const char* iw_mail)
{
 
        if ( !IW_isThere(iw_user) )
	{
	        QSqlQuery q1;
		QSqlCursor cur("iwusers");   
		QSqlRecord *buffer = cur.primeInsert();
		buffer->setValue("username",iw_user);        // cursors *in this case are fasterthan
		buffer->setValue("passwd",iw_passwd);        // queyes... and more secure coz the query
		buffer->setValue("emailaddr",iw_mail);       // do not keep Plain visible inside the binary 
		buffer->setValue("ipaddr",iw_currentip);
		cur.insert();
		q1.next();
		q1.~QSqlQuery();
		return true;
	}
	else	        	
		return false;	
}

