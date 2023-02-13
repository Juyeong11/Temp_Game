#include"pch.h"
#include"Network.h"


#include"box2D/include/box2d/box2d.h"

using namespace std;

Network network;

int main()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	int ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (ret != 0) error_display(WSAGetLastError());
	/*
	{
		Network* net = Network::GetInstance();

		net->g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN server_addr;
		ZeroMemory(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(SERVER_PORT);
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(net->g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
		listen(net->g_s_socket, SOMAXCONN);

		net->g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(net->g_s_socket), net->g_h_iocp, 0, 0);

		net->start_accept();

		vector<thread> worker_threads;

		for (int i = 0; i < 6; ++i)
			worker_threads.emplace_back(&Network::worker, net);

		for (auto& th : worker_threads)
			th.join();


	}
	*/


	//Box2D -> meters, kilograms, sec
	//Create world
	b2Vec2 gravity(0.0f, -10.0f);
	b2World world(gravity);

	//Discriptor
	b2BodyDef groundBodyDef;
	groundBodyDef.position.Set(0.0f, 0.0f);

	//Physics body
	b2Body* groundBody = world.CreateBody(&groundBodyDef);

	//Shape
	b2PolygonShape groundBox;
	groundBox.SetAsBox(50.f, 10.f);

	//Body Fixture -> groundBox
	groundBody->CreateFixture(&groundBox, 0.0f);



	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(0.0f, 4.0f);
	b2Body* body = world.CreateBody(&bodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(1.f, 1.f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.3f;

	body->CreateFixture(&fixtureDef);

	float timeStep = 1.f / 60.f;

	int velocityIterations = 6;
	int positionIterations = 2;

	for (int i = 0; i < 60; ++i) {
		world.Step(timeStep, velocityIterations, positionIterations);
		b2Vec2 position = body->GetPosition();
		float angle = body->GetAngle();

		cout << "position : " << position.x << ", " << position.y << " | angle : " << angle << endl;
	}

	WSACleanup();
}