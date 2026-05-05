#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "example_interfaces/action/sequential_sum.hpp"

class ActionServer : public rclcpp::Node 
{
	private:
		using SequentialSum = example_interfaces::action::SequentialSum;

		using Goal = SequentialSum::Goal;
		using Feedback = SequentialSum::Feedback;
		using Result = SequentialSum::Result;
		
		using GoalResponse = rclcpp_action::GoalResponse;
		using CancelResponse = rclcpp_action::CancelResponse;
		using GoalUUID = rclcpp_action::GoalUUID;
		using GoalHandle = rclcpp_action::ServerGoalHandle<SequentialSum>;
		
		rclcpp_action::Server<SequentialSum>::SharedPtr server_;

	public:
		ActionServer() : Node("action_server")
		{
			server_ = rclcpp_action::create_server<SequentialSum>(
				this,
				"/sum_numbers",
				[this](const auto& uuid, const auto& goal) {
					return this->handle_goal(uuid, goal);
				},
				[this](const auto& goal_handle) {
					return this->handle_cancel(goal_handle);
				},
				[this](const auto& goal_handle) {
					this->handle_accepted(goal_handle);
				}
			);
			
			RCLCPP_INFO(
				this->get_logger(),
				"ActionServer up & runnin'"
			);
		}

	private:

		GoalResponse handle_goal(
			const GoalUUID& uuid,
			std::shared_ptr<const Goal> goal
		)
		{
			RCLCPP_INFO(
				this->get_logger(), 
				"Received goal request: a=%f, b=%f",
				goal->a, goal->b
			);
			(void)uuid;
			return GoalResponse::ACCEPT_AND_EXECUTE;	
		}

		CancelResponse handle_cancel(const std::shared_ptr<GoalHandle> /* goal_handle */)
		{
			RCLCPP_INFO(
				this->get_logger(),
				"Received cancel request"
			);
			return CancelResponse::ACCEPT;
		}

		void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
		{
			std::thread{[this, goal_handle]() {
				this->execute(goal_handle);
			}}.detach();
		}

		void execute(const std::shared_ptr<GoalHandle> goal_handle)
		{
			auto goal = goal_handle->get_goal();
			auto feedback = std::make_shared<Feedback>();
			auto result = std::make_shared<Result>();
			
			double sum = 0;
			for (double i = goal->a; i <= goal->b; i++)
			{
				sum += i;

				feedback->current_sum = sum;
				goal_handle->publish_feedback(feedback);

				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}

			result->sum = sum;
			goal_handle->succeed(result);
		}
};

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<ActionServer>());
	rclcpp::shutdown();
	return 0;
}
