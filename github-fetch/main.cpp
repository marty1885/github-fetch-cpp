#include <drogon/drogon.h>
#include <trantor/net/EventLoopThread.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h> // C++20 foromat library (implementation in C++17)

namespace drogon
{

template<>
nlohmann::json fromRequest(const HttpRequest& req)
{
	return nlohmann::json::parse(req.body());
}

}

using namespace drogon;

Task<nlohmann::json> get_as_json(const std::string& path, const HttpClientPtr& client)
{
	auto req = HttpRequest::newHttpRequest();
	req->setPath(path);
	auto resp = co_await client->sendRequestCoro(req);
	if(resp->contentType() != CT_APPLICATION_JSON && resp->statusCode() != k200OK)
		throw std::runtime_error("Something went wrong in API request for " + path);
	co_return nlohmann::json::parse(resp->body());
}

Task<nlohmann::json> gather_user_info(const std::string& user_name, trantor::EventLoop* loop)
{
	assert(user_name.empty() == false);

	// XXX: Maybe spawn the client outside?
	auto client = HttpClient::newHttpClient("https://api.github.com/" + user_name, loop);

	nlohmann::json all_info;

	auto user_info = co_await get_as_json("/users/"+user_name, client);
	all_info["name"] = user_info["name"];
	all_info["email"] = user_info["email"];
	all_info["followers"] = user_info["followers"];
	all_info["following"] = user_info["following"];

	auto stared_repos = co_await get_as_json(fmt::format("/users/{}/starred", user_name), client);
	all_info["stars"] = stared_repos.size();

	auto repos = co_await get_as_json(fmt::format("/users/{}/repos", user_name), client);

	size_t num_forked_repos = 0;
	size_t num_public_repos = 0;
	size_t num_private_repos = 0;
	size_t num_original_repo = 0;
	std::vector<std::string> repo_list;
	repo_list.reserve(repos.size());
	for(const auto& repo : repos) {
		if(repo["private"].get<bool>())
			num_private_repos++;
		else 
			num_public_repos++;
		
		if(repo["fork"].get<bool>())
			num_forked_repos++;
		else
			num_original_repo++;
		
		repo_list.push_back(repo["full_name"].get<std::string>());
	}
	all_info["forked_repo"] = num_forked_repos;
	all_info["public_repos"] = num_public_repos;
	all_info["private_repos"] = num_private_repos;
	all_info["original_repo"] = num_original_repo;
	all_info["repo_list"] = repo_list;

	// TODO: Fetch repo info

	co_return all_info;
}

Task<> print_user_info(const std::string_view& profile_url, trantor::EventLoop* loop, bool json=false)
{
	assert(profile_url.empty() == false);
	assert(loop != nullptr);

	// Quick hack to get me going
	std::string username = std::string(profile_url);
	utils::replaceAll(username, "https://github.com/", "");

	auto user_info = co_await gather_user_info(username, loop);

	if(json) {
		std::cout << user_info.dump() << std::endl;
		co_return;
	}
	
	std::string email = user_info["email"].is_null() ? std::string("<No Info>") : user_info["email"].get<std::string>();
	std::cout << fmt::format(
		"Name: {}\n"
		"Email: {}\n"
		"Built: {} Forked: {} Stars: {} Followers: {} Following: {}\n"
		"\n"
		, user_info["name"].get<std::string>()
		, email
		, user_info["original_repo"].get<size_t>()
		, user_info["forked_repo"].get<size_t>()
		, user_info["stars"].get<size_t>()
		, user_info["followers"].get<size_t>()
		, user_info["following"].get<size_t>()
		, user_info["public_repos"].get<size_t>()
		, user_info["private_repos"].get<size_t>());
	

	std::cout << "Repositories:\n";
	for(auto repo : user_info["repo_list"]) {
		std::string full_name = repo.get<std::string>();
		std::cout << fmt::format("  {}\n\n", full_name);
	}
}

int main(int argc, char** argv)
{
	// HACK: Nasty command line parser
	bool json = false;
	for(int i=1;i<argc;i++) {
		std::string arg = argc[i];
		if(arg == "--json")
			json = true;
		else if(arg == "-h") {
			std::cout << "Usage " << argv[0] << " [--json]\n";
			exit(0);
		}
	}
	trantor::EventLoopThread thread;
	thread.run();
	sync_wait(print_user_info("https://github.com/x724", thread.getLoop(), false));
	// EventLoopThread joins automatically
}
