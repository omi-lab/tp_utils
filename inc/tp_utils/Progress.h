#ifndef tp_utils_Progress_h
#define tp_utils_Progress_h

#include "tp_utils/CallbackCollection.h"

namespace tp_utils
{
class AbstractCrossThreadCallbackFactory;

//##################################################################################################
struct ProgressMessage
{
  std::string message;
  bool error{false};
  size_t indentation{0};

  ProgressMessage() = default;

  ProgressMessage(const std::string& message_, bool error_, size_t indentation_):
    message(message_),
    error(error_),
    indentation(indentation_)
  {

  }
};

//##################################################################################################
//! A class for recording the progress of an operation.
/*!

*/
class TP_UTILS_SHARED_EXPORT Progress
{
public:
  //################################################################################################
  //! Thread safe constructor.
  Progress(AbstractCrossThreadCallbackFactory* crossThreadCallbackFactory);

  //################################################################################################
  //! Blocking operation constructor.
  Progress(const std::function<bool()>& poll);

  //################################################################################################
  //! Child step constructor.
  Progress(Progress* parent);

  //################################################################################################
  virtual ~Progress();

  //################################################################################################
  void setPrintToConsole(bool printToConsole);

  //################################################################################################
  //! Set the progress
  /*!
  \param fraction of the way through this task (0 to 1)
  */
  void setProgress(float fraction);

  //################################################################################################
  //! Set the progress and the description of what is being processes
  /*!
  \param fraction of the way through this task (0 to 1)
  */
  void setProgress(float fraction, const std::string& description);

  //################################################################################################
  //! Returns the current progress as a fraction
  float progress() const;

  //################################################################################################
  //! Sets the description of the task that is being performed
  /*!
  This should be updated to describe what is happening at each stage of a process, this should be a
  short string that will be hisplayed to the user.

  \param description - The new description
  */
  void setDescription(const std::string& description);

  //################################################################################################
  //! Returns the descripton of the current stage of the task.
  std::string description() const;

  //################################################################################################
  /*!
  \param description of the child step.
  \param completeFraction the progress of this once the child step is complete.
  \return A pointer to new Progress object, owned by this.
  */
  Progress* addChildStep(const std::string& message, float completeFraction);

  //################################################################################################
  //! Log a message that will be kept in the tree of messages.
  void addMessage(const std::string& message);

  //################################################################################################
  //! Log a message that will be kept in the tree of messages.
  void addError(const std::string& error);

  //################################################################################################
  //! Return the tree of messages.
  std::vector<ProgressMessage> allMessages() const;

  //################################################################################################
  //! Return the tree of error messages.
  std::vector<ProgressMessage> errorMessages() const;

  //################################################################################################
  std::string compileErrors() const;

  //################################################################################################
  std::vector<Progress*> childSteps() const;

  //################################################################################################
  //! Returs true if this task should stop
  bool shouldStop() const;

  //################################################################################################
  //! Use this to stop the task
  void stop(bool shouldStop);

  //################################################################################################
  bool poll();

  //################################################################################################
  CallbackCollection<void()> changed;

protected:
  //################################################################################################
  void callChanged();

  //################################################################################################
  void getAllMessages(size_t indentation, std::vector<ProgressMessage>& messages) const;

  //################################################################################################
  bool getErrors(size_t indentation, std::vector<ProgressMessage>& messages) const;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
