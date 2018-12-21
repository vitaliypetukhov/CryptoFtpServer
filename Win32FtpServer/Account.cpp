#pragma once
#include "Account.h"

Account::Account()
{
	Login = "Anonymous";
	Password = "Anonymous@mail.com";
	HomePath = "C:\\";
}
Account::Account(vector<string> tokens)
{
	Login = tokens[0];
	Password = tokens[1];
	HomePath = tokens[2];
}
Account::Account(string Login, string Password, string HomePath)
{
	this->Login = Login;
	this->Password = Password;
	this->HomePath = HomePath;
}
Account::Account(const Account &href)
{
	Login = href.Login;
	Password = href.Password;
	HomePath = href.HomePath;
}
Account::~Account() {}
string Account::GetLogin() const
{
	return Login;
}
void Account::SetLogin(string Login)
{
	this->Login = Login;
}
string Account::GetPassword() const
{
	return Password;
}
void Account::SetPassword(string Password)
{
	this->Password = Password;
}
string Account::GetHome() const
{
	return HomePath;
}
void Account::SetHome(string HomePath)
{
	this->HomePath = HomePath;
}
