#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "example_interfaces/action/sequential_sum.hpp"

class ActionClient : public rclcpp::Node 
{
	private:
		using SequentialSum = example_interfaces::action::SequentialSum;
		using Feedback = SequentialSum::Feedback;
		using Goal = SequentialSum::Goal;
		
		using GoalHandle = rclcpp_action::ClientGoalHandle<SequentialSum>;
		using ResultCode = rclcpp_action::ResultCode;

		using Result = GoalHandle::WrappedResult;
		
		rclcpp::TimerBase::SharedPtr timer_;
		rclcpp_action::Client<SequentialSum>::SharedPtr client_;
		rclcpp_action::Client<SequentialSum>::SendGoalOptions options;
		
	public:
		bool done = false;
			
		ActionClient() : Node("action_client")
		{
			client_ = rclcpp_action::create_client<SequentialSum>(
				this,
				"/sum_numbers"
			);

			timer_ = this->create_wall_timer(
				std::chrono::milliseconds(500),
				[this]() {
					this->on_timer();
				}
			);

			RCLCPP_INFO(
				this->get_logger(),
				"Action Client up & runnin'"
			);
		}

	private:
		void on_timer()
		{
			if (!client_->action_server_is_ready()) {
				RCLCPP_WARN(
					this->get_logger(), 
					"Waiting for server..."
				);
				return;
			}
			
			timer_->cancel();
			RCLCPP_INFO(
				this->get_logger(),
				"Server found."
			);
				
			double a, b;
			std::cout << "Start of sequence: ";
			std::cin >> a;
			std::cout << '\n' << "End of sequence: ";
			std::cin >> b;

			this->send_goal(a, b);
		}

		void send_goal(double a, double b)
		{
			auto goal_msg = Goal();
			goal_msg.a = a;
			goal_msg.b = b;

			RCLCPP_INFO(
				this->get_logger(),
				"Sending goal"
			);

			options.goal_response_callback = [this](const auto& goal_handle) {
				this->on_goal_response(goal_handle);
			};

			options.feedback_callback = [this](const auto& goal_handle, const auto& feedback) {
				this->on_feedback(goal_handle, feedback);
			};

			options.result_callback = [this](const auto& result) {
				this->on_result(result);
			};

			client_->async_send_goal(goal_msg, options);
		}

		void on_goal_response(std::shared_ptr<GoalHandle> goal_handle)
		{
			if (!goal_handle) {
				RCLCPP_ERROR(this->get_logger(), "Goal rejected by server.");
			}
			else {
				RCLCPP_INFO(this->get_logger(), "Goal accepted by server. Waiting for result...");
			}
		}

		void on_feedback(
			std::shared_ptr<GoalHandle> goal_handle,
			const std::shared_ptr<const Feedback> feedback
		)
		{
			std::string uuid = rclcpp_action::to_string(goal_handle->get_goal_id());
		
			RCLCPP_INFO(
				this->get_logger(), 
				"Feedback received for goal with UUID: %s",
				uuid.c_str()
			);
			
			RCLCPP_INFO(
				this->get_logger(),
				"Current sum: %f",
				feedback->current_sum
			);
		}

		void on_result(const Result& result)
		{
			switch (result.code)
			{
				case ResultCode::SUCCEEDED:
					RCLCPP_INFO(
						this->get_logger(),
						"Success. Resulting sum: %f",
						result.result->sum
					);
					break;
				
				case ResultCode::ABORTED:
					RCLCPP_ERROR(
						this->get_logger(),
						"Goal aborted."
					);
					break;
				
				/*
				some time was spent to decide whether
				CANCELED is INFO or WARN related

				CANCELED does not mean the object is in a
				dangerous state, since it's just a sequential
				summator, therefore WARN is unrelated
				*/
				case ResultCode::CANCELED:
					RCLCPP_INFO(
						this->get_logger(),
						"Goal canceled."
					);
					break;
				
				default:
					RCLCPP_ERROR(
						this->get_logger(),
						"Unknown result code."
					);
					break;
			}

			this->done = true;
		}
};

int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);

	auto node = std::make_shared<ActionClient>();
	while (rclcpp::ok() && !node->done) {
		rclcpp::spin_some(node);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	rclcpp::shutdown();
}

/*
Oh how much time was spent to perfect the code
*/
