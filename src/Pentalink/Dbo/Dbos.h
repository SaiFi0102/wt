#ifndef TP_DBOS_H
#define TP_DBOS_H

#include "Utilities/Common.h"
#include <Wt/Dbo/Dbo>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/WDate>

#define TRANSACTION(app) Wt::Dbo::Transaction t(app->dboSession())

namespace PL
{
	void mapDboTree(Wt::Dbo::Session &dboSession);

	//User related
	class User;
	typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
	typedef Wt::Auth::Dbo::AuthIdentity<AuthInfo> AuthIdentity;
	typedef Wt::Auth::Dbo::AuthToken<AuthInfo> AuthToken;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<AuthInfo>> AuthInfoCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<User>> UserCollection;

	typedef Wt::Auth::Dbo::UserDatabase<PL::AuthInfo> UserDatabase;

	//Game related
	class Game;
	struct PlayerPK;
	class Player;
	class GameMove;

	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Game>> GameCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<Player>> PlayerCollection;
	typedef Wt::Dbo::collection<Wt::Dbo::ptr<GameMove>> GameMoveCollection;
}

namespace Wt
{
	namespace Dbo
	{
		template <class Action>
		void field(Action &a, PL::PlayerPK &key, const std::string &, int size = -1)
		{
			Wt::Dbo::belongsTo(a, key.gamePtr, "game", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
			Wt::Dbo::belongsTo(a, key.userPtr, "user", Wt::Dbo::OnDeleteCascade | Wt::Dbo::OnUpdateCascade | Wt::Dbo::NotNull);
		}
		template<>
		struct dbo_traits<PL::Player> : public dbo_default_traits
		{
			typedef PL::PlayerPK IdType;
			static IdType invalidId();
			constexpr static const char *surrogateIdField() { return nullptr; }
		};

		template<>
		struct dbo_traits<PL::GameMove> : public dbo_default_traits
		{
			typedef Wt::Dbo::ptr<PL::Player> IdType;
			static IdType invalidId() { return IdType(); }
			constexpr static const char *surrogateIdField() { return nullptr; }
		};
	}
}

namespace PL
{
	
	class User
	{
	public:
		Wt::Dbo::weak_ptr<AuthInfo> authInfoWPtr;
		PlayerCollection playerCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::hasOne(a, authInfoWPtr, "user");
			Wt::Dbo::hasMany(a, playerCollection, Wt::Dbo::ManyToOne, "user");
		}
		static const char *tableName()
		{
			return "user";
		}
	};

	class Game
	{
	public:
		enum State
		{
			InGame = 0,
			Finished = 1
		};

		int rows;
		int columns;
		int nConsecutiveLink = 4;
		State state;
		Wt::Dbo::ptr<User> winnerUserPtr;

		PlayerCollection playerCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::field(a, rows, "rows");
			Wt::Dbo::field(a, columns, "columns");
			Wt::Dbo::field(a, nConsecutiveLink, "nConsecutiveLink");
			Wt::Dbo::field(a, state, "state");
			Wt::Dbo::belongsTo(a, winnerUserPtr, "winner_user", Wt::Dbo::OnDeleteSetNull | Wt::Dbo::OnUpdateCascade);

			Wt::Dbo::hasMany(a, playerCollection, Wt::Dbo::ManyToOne, "game");
		}
		static const char *tableName()
		{
			return "game";
		}
	};

	struct PlayerPK
	{
		Wt::Dbo::ptr<Game> gamePtr;
		Wt::Dbo::ptr<User> userPtr;

		PlayerPK() = default;
		PlayerPK(Wt::Dbo::ptr<Game> gamePtr, Wt::Dbo::ptr<User> userPtr)
			: userPtr(userPtr), gamePtr(gamePtr)
		{ }

		bool operator==(const PlayerPK &other) const
		{
			return userPtr == other.userPtr && gamePtr == other.gamePtr;
		}

		bool operator<(const PlayerPK &other) const
		{
			if(userPtr < other.userPtr)
				return true;
			else if(userPtr == other.userPtr)
				return gamePtr < other.gamePtr;
			else
				return false;
		}
	};

	class Player
	{
	public:
		Player() = default;
		Player(Wt::Dbo::ptr<Game> gamePtr, Wt::Dbo::ptr<User> userPtr) : _id(gamePtr, userPtr) { }

		GameMoveCollection gameMoveCollection;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _id, "id");
			Wt::Dbo::hasMany(a, gameMoveCollection, Wt::Dbo::ManyToOne, "player");
		}
		static const char *tableName()
		{
			return "player";
		}

	protected:
		PlayerPK _id;
	};
	std::ostream &operator<<(std::ostream &o, const PlayerPK &k);

	class GameMove
	{
	public:
		GameMove() = default;
		GameMove(Wt::Dbo::ptr<Player> playerPtr) : _playerPtr(playerPtr) { }

		int row;
		int column;

		template<class Action>
		void persist(Action& a)
		{
			Wt::Dbo::id(a, _playerPtr, "player");
			Wt::Dbo::field(a, row, "row");
			Wt::Dbo::field(a, column, "column");
		}
		static const char *tableName()
		{
			return "game_move";
		}

	protected:
		Wt::Dbo::ptr<Player> _playerPtr;
	};

}

#endif
