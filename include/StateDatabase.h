/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_STATE_DATABASE_H
#define BZF_STATE_DATABASE_H

#include "common.h"
#include <string>
#include <vector>
#include "CallbackList.h"
#include "bzfio.h"

#define BZDB (StateDatabase::getInstance())

class StateDatabase {
public:
	typedef void (*Callback)(const std::string& name, void* userData);
	enum Permission {
		// permission levels
		ReadWrite,
		Locked,
		ReadOnly,

		// access levels
		User   = ReadWrite,
		Server = Locked,
		Client = ReadOnly
	};

	~StateDatabase();

	// set a name/value pair.  if access is less than the permission
	// level of the name then this has no effect.
	void				set(const std::string& name, const std::string& value,
							Permission access = Client);

	// unset a name if access is not less than the permission level
	// of the name.
	void				unset(const std::string& name,
							Permission access = Client);

	// simulate a change to a value (i.e. invoke the callbacks on it)
	void				touch(const std::string& name,
							Permission access = Client);

	// mark a value as persistent (i.e. to be saved) or volatile.
	// this state is stored independently of the existance of a value
	// with the given name.  that is, adding or removing the name
	// will not affect persistence of the name.  the default is
	// volatile.
	void				setPersistent(const std::string& name, bool = true);

	// set the default value for a name.  if the default value is set
	// then the value will not be written by write() if the current
	// value is equal to the default value.
	void				setDefault(const std::string& name,
							const std::string& value);

	// set the permission level of a name.  like persistence, this is
	// stored independently of a value with the name.  the default
	// permission is ReadWrite (i.e. full access).
	void				setPermission(const std::string& name, Permission);

	// add/remove a callback to/from a name.  all callbacks on a name are
	// invoked when the value changes (either by being set or unset).
	// each name can have any number of callbacks but any given callback
	// function/userData pair on a name can only be registered once (i.e.
	// multiple adds have the same effect as a single add).
	void				addCallback(const std::string& name,
							Callback, void* userData);
	void				removeCallback(const std::string& name,
							Callback, void* userData);

	// test if a name is set or not
	bool				isSet(const std::string& name) const;

	// get the value associated with a name.  returns the empty string
	// if the name isn't set.
	std::string			get(const std::string& name) const;

	// get the value as a floating point number. this will evaluate
	// the string as an expression
	float				eval(std::string name) const;

	// return true if the value associated with a name indicates
	// logical true, which is when the value is not empty and not
	// "0" and not "false" and not "no".
	bool				isTrue(const std::string& name) const;

	// test if a name is empty or not.  a name is empty if it's
	// not set or it's set to the empty string.
	bool				isEmpty(const std::string& name) const;

	// get the persistence, permission, and default for an entry
	bool				isPersistent(const std::string& name) const;
	std::string			getDefault(const std::string& name) const;
	Permission			getPermission(const std::string& name) const;

	// invoke the callback for each entry
	void				iterate(Callback, void* userData) const;

	// invoke the callback for each entry that should be written (i.e.
	// is set, persistent, and not the default).
	void				write(Callback, void* userData) const;

	// get the singleton instance of the state database
	static StateDatabase* getInstance();

private:
	StateDatabase();

	static bool			onCallback(Callback, void* userData, void* iterateData);

	struct Item {
	public:
		Item();

	public:
		std::string		value;
		std::string		defValue;
		bool			isSet;
		bool			isTrue;
		bool			save;
		Permission		permission;
		CallbackList<Callback>	callbacks;
	};
	typedef std::map<std::string, Item> Map;

	Map::iterator		lookup(const std::string&);
	void				notify(Map::iterator);

private:
	Map					items;
	static StateDatabase* s_instance;

public:
	class ExpressionToken {
	public:
		enum Type { oper, number, variable };
		enum Operator { add, subtract, multiply, divide, power, lparen, rparen, none };
		struct Contents {
		public:
			double number;
			std::string variable;
			Operator oper;
		};

		ExpressionToken();
		ExpressionToken(Type _tokenType);
		ExpressionToken(Type _tokenType, Contents _tokenContents);

		void				setType(Type _tokenType);
		void				setContents(Contents _tokenContents);
		void				setNumber(double number);
		void				setVariable(std::string variable);
		void				setOper(Operator oper);

		Type				getTokenType();
		Contents			getTokenContents();
		double				getNumber();
		std::string			getVariable();
		Operator			getOperator();

		int					getPrecedence();

		friend std::istream&		operator >> (std::istream& src, ExpressionToken& dst);
		friend std::string&	operator >> (std::string& src, ExpressionToken& dst);
		friend std::ostream&		operator << (std::ostream& dst, ExpressionToken& src);
	private:
		Type tokenType;
		Contents tokenContents;
	};

	typedef std::vector<ExpressionToken> Expression;

private:
	Expression				infixToPrefix(Expression infix) const;
	float					evaluate(Expression e) const;
};

std::istream& operator >> (std::istream& src, StateDatabase::Expression& dst);
std::string& operator >> (std::string& src, StateDatabase::Expression& dst);
std::ostream& operator << (std::ostream& dst, StateDatabase::Expression& src);

#endif // BZF_STATE_DATABASE_H
// ex: shiftwidth=4 tabstop=4
