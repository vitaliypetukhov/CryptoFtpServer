#pragma once
#include <string>
#include <vector>
using namespace std;

class Account
{
public:
	Account();
	Account(string, string, string);
	Account(vector<string>);
	Account(const Account &href);
	~Account();
	string GetLogin() const;
	void SetLogin(string);
	string GetPassword() const;
	void SetPassword(string);
	string GetHome() const;
	void SetHome(string);
private:
	string Login, Password, HomePath;
};