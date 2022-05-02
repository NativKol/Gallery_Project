#include <map>
#include <algorithm>

#include "ItemNotFoundException.h"
#include "MemoryAccess.h"

//stats
int numSave = 0;

//get the id
int albumCount = 0;
int tagCount = 0;

//does exists
bool albumExist = false;
bool userExist = false;

//album
std::string albumID = "";
std::string album_date = "";
std::string owner_id = "";
std::string album_name = "";

//picture
std::string pictureID = "";
std::string pic_name = "";
std::string pic_path = "";
std::string pic_date = "";

//user
std::string userID;
std::string username = "";

//lists
std::list<Album> albums_topen;
std::list<Picture> pics_topen;

// **************** DATABASE ****************

//DONE
bool MemoryAccess::open()
{
	//sqlite3* db;
	std::string dbFileName = "musicDB.sqlite";
	int file_exist = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &db);
	if (res != SQLITE_OK) {
		db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return -1;
	}
	if (file_exist == 0)
	{
		//close db
		std::cout << "The file already exists" << std::endl;
	}
	else if (file_exist != 0)
	{

		// init database
		std::cout << "Create new database.." << std::endl;

		char** errMessage = nullptr;
		const char* sqlStatement;

		
		//Create Users Table
		sqlStatement = R""""""(
				CREATE TABLE USERS (ID INTEGER PRIMARY
				KEY AUTOINCREMENT NOT NULL , NAME
				TEXT NOT NULL);
				)"""""";

		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
		if (res != SQLITE_OK)
			return false;

		//Create Albums Table
		sqlStatement = R""""""(
				CREATE TABLE ALBUMS (ID INTEGER PRIMARY
				KEY AUTOINCREMENT NOT NULL , NAME
				TEXT NOT NULL, CREATION_DATE TEXT NOT NULL,
				USER_ID INTEGER NOT NULL,
				FOREIGN KEY(USER_ID) REFERENCES USERS(ID));
				)"""""";

		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
		if (res != SQLITE_OK)
			return false;

		//Create Pictures Table
		sqlStatement = R""""""(
				CREATE TABLE PICTURES (ID INTEGER PRIMARY
				KEY AUTOINCREMENT NOT NULL , NAME
				TEXT NOT NULL,LOCATION TEXT NOT NULL,
				CREATION_DATE TEXT NOT NULL,	
				ALBUM_ID INTEGER NOT NULL,
				FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));
				)"""""";

		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
		if (res != SQLITE_OK)
			return false;

		//Create Tags Table
		sqlStatement = R""""""(
				CREATE TABLE Tags (
				ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, 
				PICTURE_ID INTEGER NOT NULL,
				USER_ID INTEGER NOT NULL,
				FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID),
				FOREIGN KEY(USER_ID) REFERENCES USERS(ID));
				)"""""";

		res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
		if (res != SQLITE_OK)
			return false;

	}
}		

//DONE
void MemoryAccess::close()
{
	sqlite3_close(db);
	db = nullptr;

}

//DONE
void MemoryAccess::clear()
{
	m_users.clear();
	m_albums.clear();
}

// ******************* Album ******************* 

int callback_printAlbum(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << ", ";
	}
	std::cout << std::endl;
	return 0;
}

//DONE
void MemoryAccess::printAlbums()
{
	std::cout << "Albums list:" << std::endl;
	std::cout << "-------------" << std::endl;
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT * FROM Albums;";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_printAlbum, nullptr, errMessage);
}


//no need
auto MemoryAccess::getAlbumIfExists(const std::string& albumName)
{
	auto result = std::find_if(std::begin(m_albums), std::end(m_albums), [&](auto& album) { return album.getName() == albumName; });

	if (result == std::end(m_albums)) {
		throw ItemNotFoundException("Album not exists: ", albumName);
	}
	return result;
}

//no need
Album MemoryAccess::createDummyAlbum(const User& user)
{
	std::stringstream name("Album_" + std::to_string(user.getId()));

	Album album(user.getId(), name.str());

	for (int i = 1; i < 3; ++i) {
		std::stringstream picName("Picture_" + std::to_string(i));

		Picture pic(i++, picName.str());
		pic.setPath("C:\\Pictures\\" + picName.str() + ".bmp");

		album.addPicture(pic);
	}

	return album;
}

int callback_createPicList(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "ID")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pictureID = argv[i];
		}
		else if (std::string(azColName[i]) == "NAME")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_name = argv[i];
		}
		else if (std::string(azColName[i]) == "LOCATION")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_path = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATION_DATE")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_date = argv[i];
			pics_topen.push_back(Picture(std::stoi(pictureID), pic_name, pic_path, pic_date));
		}
	}
	std::cout << std::endl;
	return 0;
}

int callback_createAlbum(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "USER_ID")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			owner_id = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATION_DATE")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			album_date = argv[i];
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
Album MemoryAccess::openAlbum(const std::string& albumName)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;
	pics_topen.clear();
	albumID = getAlbumID(albumName);

	sqlCommand = "SELECT * FROM PICTURES WHERE ALBUM_ID == " + albumID + ";";

	const char* sqlStatement = sqlCommand.c_str();


	res = sqlite3_exec(db, sqlStatement, callback_createPicList, nullptr, errMessage);

	sqlCommand = "SELECT * FROM ALBUMS WHERE NAME == \"" + albumName + "\";";

	sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_createAlbum, nullptr, errMessage);

	Album a1(std::stoi(owner_id), albumName, album_date);
	a1.addPictures(pics_topen);
	return a1;

}

int callback_createAlbumsList(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "NAME")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			album_name = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATION_DATE")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			album_date = argv[i];
		}
		else if (std::string(azColName[i]) == "USER_ID")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			owner_id = argv[i];
			albums_topen.push_back(Album(std::stoi(owner_id), album_name, album_date));
		}
		std::cout  << std::endl;
	}
	//std::cout << std::endl;
	return 0;
}

//DONE
const std::list<Album> MemoryAccess::getAlbums()
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;
	albums_topen.clear();

	sqlCommand = "SELECT * FROM ALBUMS;";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_createAlbumsList, nullptr, errMessage);

	return albums_topen;
}

//DONE
const std::list<Album> MemoryAccess::getAlbumsOfUser(const User& user)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;
	albums_topen.clear();

	sqlCommand = "SELECT * FROM ALBUMS WHERE USER_ID == " + std::to_string(user.getId()) + ";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_createAlbumsList, nullptr, errMessage);

	return albums_topen;
}

//DONE
void MemoryAccess::createAlbum(const Album& album)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	albumCount++;

	sqlCommand = "INSERT INTO ALBUMS VALUES(" + std::to_string(albumCount) + ", \"" + album.getName() +
		"\", \"" + album.getCreationDate() + "\", "  + std::to_string(album.getOwnerId()) + "); ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

//DONE
void MemoryAccess::deleteAlbum(const std::string& albumName, int userId)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	albumCount--;

	sqlCommand = "DELETE FROM Albums WHERE name == \"" + albumName + "\"; ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

int callback_existAlbum(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "count(ID)")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			if (*argv[i] == '0')
			{
				albumExist = false;
			}
			else
			{
				albumExist = true;
			}
			return 0;
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
bool MemoryAccess::doesAlbumExists(const std::string& albumName)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT count(ID) FROM ALBUMS WHERE name == \"" + 
		albumName + "\";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_existAlbum, nullptr, errMessage);
	return albumExist;
}

int callback_AlbumID(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "ID")
		{
			if (std::string(azColName[i]) == "ID")
			{
				std::cout << azColName[i] << " = " << argv[i] << std::endl;
				albumID = argv[i];
				return 0;
			}
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
std::string MemoryAccess::getAlbumID(const std::string& albumName)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT ID FROM ALBUMS WHERE name == \"" + albumName + "\"; ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_AlbumID, nullptr, errMessage);
	return albumID;
}

//DONE
void MemoryAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{		
	albumID = getAlbumID(albumName);
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "INSERT INTO Pictures VALUES(" + std::to_string(picture.getId()) + ", \"" +
		picture.getName() + "\", \"" + picture.getPath() +"\", \"" +
		picture.getCreationDate() +"\", " + albumID + "); ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

//DONE
void MemoryAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	albumID = getAlbumID(albumName);
	pictureID = getPictureID(pictureName, albumID);
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	untagUserInPicture(albumName, pictureName, 0);		// 0 - Not spesific user

	sqlCommand = "DELETE FROM PICTURES WHERE ID == " + pictureID + ";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

int callback_UserID(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << " , ";
	}
	std::cout << std::endl;
	return 0;
}

//DONE
void MemoryAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	albumID = getAlbumID(albumName);
	pictureID = getPictureID(pictureName, albumID);

	tagCount++;

	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "INSERT INTO Tags VALUES(" +std::to_string(tagCount) +", " + pictureID + ", " + std::to_string(userId) +");";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

//DONE
void MemoryAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	albumID = getAlbumID(albumName);
	pictureID = getPictureID(pictureName, albumID);

	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	tagCount--;

	sqlCommand = "DELETE FROM Tags WHERE PICTURE_ID == " + pictureID +
		" OR USER_ID == " + std::to_string(userId) + ";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

int callback_PictureID(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "ID")
		{
			if (std::string(azColName[i]) == "ID")
			{
				std::cout << azColName[i] << " = " << argv[i] << std::endl;
				pictureID = argv[i];
				return 0;
			}
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
std::string MemoryAccess::getPictureID(const std::string& pictureName, std::string album_ID)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT ID FROM Pictures WHERE name == \"" + pictureName +
		"\" and ALBUM_ID == " + album_ID +"; ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_PictureID, nullptr, errMessage);
	return pictureID;
}

//DONE
void MemoryAccess::closeAlbum(Album&)
{
	// basically here we would like to delete the allocated memory we got from openAlbum
}

// ******************* User ******************* 

int callback_printUser(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << ", ";
	}
	std::cout << std::endl;
	return 0;
}

//DONE
void MemoryAccess::printUsers()
{
	std::cout << "Users list:" << std::endl;
	std::cout << "-----------" << std::endl;
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT * FROM USERS;";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_printUser, nullptr, errMessage);

}

int callback_NameUser(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "NAME")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			username = argv[0];
			return 0;
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
User MemoryAccess::getUser(int userId) 
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT NAME FROM USERS WHERE ID == " + std::to_string(userId) + ";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_NameUser, nullptr, errMessage);

	User u1(userId, username);
	return u1;
}

//DONE
void MemoryAccess::createUser(User& user)
{

	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	 sqlCommand = "INSERT INTO Users VALUES(" + std::to_string(user.getId()) + ", \" "+ user.getName() + "\"); ";

	 const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

//DONE
void MemoryAccess::deleteUser(const User& user)
{

	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "DELETE FROM Users WHERE ID == " + std::to_string(user.getId()) + "; ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;

	sqlCommand = "DELETE FROM Albums WHERE USER_ID == " + std::to_string(user.getId()) + "; ";
	sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, nullptr, nullptr, errMessage);
	if (res != SQLITE_OK)
		return;
}

int callback_existUser(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "count(ID)")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			if (*argv[i] == '0')
			{
				userExist = false;
			}
			else
			{
				userExist = true;
			}
			return 0;
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
bool MemoryAccess::doesUserExists(int userId)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT count(ID) FROM USERS WHERE ID == " + std::to_string(userId) + ";";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_existUser, nullptr, errMessage);
	return userExist;
}


// **************** user statistics  - TODO *********************

int callback_countOwned(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << std::endl;
		numSave = std::stoi(argv[i]);
	}
	std::cout << std::endl;
	return 0;
}
//DONE
int MemoryAccess::countAlbumsOwnedOfUser(const User& user)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT count(ID) AS AmountOfAlbums FROM ALBUMS where USER_ID = " + std::to_string(user.getId()) + ";" ;

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_countOwned, nullptr, errMessage);
	return numSave;
}
//DONE
int MemoryAccess::countAlbumsTaggedOfUser(const User& user)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT count(DISTINCT ALBUM_ID) AS ALBUMS_TAGGED FROM Tags JOIN PICTURES ON PICTURES.ID = TAGS.PICTURE_ID WHERE USER_ID == " + 
		std::to_string(user.getId()) + "; ";

	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_countOwned, nullptr, errMessage);
	return numSave;
}

//DONE
int MemoryAccess::countTagsOfUser(const User& user)
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT count(USER_ID) AS COUNT_TAGS FROM Tags where USER_ID == " + std::to_string(user.getId()) + ";";
	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_countOwned, nullptr, errMessage);
	return numSave;
}
//NO NEED
float MemoryAccess::averageTagsPerAlbumOfUser(const User& user)
{
	int albumsTaggedCount = countAlbumsTaggedOfUser(user);

	if (0 == albumsTaggedCount) {
		return 0;
	}

	return static_cast<float>(countTagsOfUser(user)) / albumsTaggedCount;
}

// *********************** Querirs **************************

int callback_topUser(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		std::cout << azColName[i] << " = " << argv[i] << std::endl;
		if (std::string(azColName[i]) == "USER_ID")
		{
			userID = argv[i];
		}
		else if (std::string(azColName[i]) == "NAME")
		{
			username = argv[i];
			return 0;
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
User MemoryAccess::getTopTaggedUser()
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT USER_ID, name, count(USER_ID) AS MOST_TAGGED FROM Tags JOIN USERS ON USERS.ID == TAGS.USER_ID GROUP BY USER_ID ORDER BY MOST_TAGGED ASC; ";
	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_topUser, nullptr, errMessage);
	return User(std::stoi(userID), username);
}

int callback_topPic(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "PICTURE_ID")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pictureID = argv[i];
		}
		else if (std::string(azColName[i]) == "NAME")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_name = argv[i];
		}
		else if (std::string(azColName[i]) == "LOCATION")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_path = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATION_DATE")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_date = argv[i];
			return 0;
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
Picture MemoryAccess::getTopTaggedPicture()
{
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT *, count(PICTURE_ID) AS MOST_TAGGED FROM Tags JOIN PICTURES ON TAGS.PICTURE_ID = PICTURES.ID GROUP BY PICTURE_ID ORDER BY MOST_TAGGED ASC; ";
	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_topPic, nullptr, errMessage);
	return Picture(std::stoi(pictureID), pic_name, pic_path, pic_date);
}

int callback_topPicByUser(void* data, int argc, char** argv, char** azColName)
{
	for (int i = 0; i < argc; i++)
	{
		if (std::string(azColName[i]) == "PICTURE_ID")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pictureID = argv[i];
		}
		else if (std::string(azColName[i]) == "NAME")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_name = argv[i];
		}
		else if (std::string(azColName[i]) == "LOCATION")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_path = argv[i];
		}
		else if (std::string(azColName[i]) == "CREATION_DATE")
		{
			std::cout << azColName[i] << " = " << argv[i] << std::endl;
			pic_date = argv[i];
			pics_topen.push_back(Picture(std::stoi(pictureID), pic_name, pic_path, pic_date));
		}
	}
	std::cout << std::endl;
	return 0;
}

//DONE
std::list<Picture> MemoryAccess::getTaggedPicturesOfUser(const User& user)
{
	pics_topen.clear();
	char** errMessage = nullptr;
	std::string sqlCommand;
	int res;

	sqlCommand = "SELECT *, count(PICTURE_ID) AS MOST_TAGGED FROM Tags JOIN PICTURES ON TAGS.PICTURE_ID = PICTURES.ID WHERE USER_ID == " + std::to_string(user.getId()) + " GROUP BY PICTURE_ID ORDER BY MOST_TAGGED ASC; ";
	const char* sqlStatement = sqlCommand.c_str();

	res = sqlite3_exec(db, sqlStatement, callback_topPicByUser, nullptr, errMessage);
	return pics_topen;
}