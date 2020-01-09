#include "complex_graph_cell.h"

using namespace Shared;
using namespace Poincare;

namespace Calculation {

ComplexGraphView::ComplexGraphView(ComplexModel * complexModel) :
  CurveView(complexModel),
  m_complex(complexModel)
{
}

void ComplexGraphView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(rect, KDColorWhite);

  // Draw grid, axes and graduations
  drawGrid(ctx, rect);
  drawAxes(ctx, rect);
  drawLabelsAndGraduations(ctx, rect, Axis::Vertical, true);
  drawLabelsAndGraduations(ctx, rect, Axis::Horizontal, true);

  float real = m_complex->real();
  float imag = m_complex->imag();

  /* Draw the segment from the origin to the dot (real, imag) of equation
   * x(t) = t*real and y(t) = t*imag with t in [0,1] */
  drawCurve(ctx, rect, 0.0f, 1.0f, 0.01f,
      [](float t, void * model, void * context) {
        ComplexModel * complexModel = (ComplexModel *)model;
        return Poincare::Coordinate2D<float>(complexModel->real()*t, complexModel->imag()*t);
      }, m_complex, nullptr, false, Palette::GreyDark, false);

  /* Draw the partial ellipse indicating the angle θ
   * - the ellipse parameters are a = |real|/5 and b = |imag|/5,
   * - the parametric ellipse equation is x(t) = a*cos(th*t) and y(t) = b*sin(th*t)
   *   with th computed in order to be the intersection of the line forming an
   *   angle θ with the abscissa and the ellipsis
   * - we draw the ellipse for t in [0,1] to represent it from the abscissa axis
   *   to the phase of the complex
   */
  /* Compute th: th is the intersection of ellipsis of equation (a*cos(t), b*sin(t))
   * and the line of equation (t,t*tan(θ)).
   * (a*cos(t), b*sin(t)) = (t,t*tan(θ)) --> t = arctan((a/b)*tan(θ)) (± π) */
  assert(imag != 0.0f); // ComplexGraphView is not displayed for pure real
  float th = std::atan(std::fabs(real/imag)*std::tan(std::arg(*m_complex)));
  if (real < 0.0f) {
    th += imag < 0.0f ? -M_PI : M_PI; // atan returns value in [-π/2,π/2]
  }
  // Compute ellipsis parameters a and b
  float factor = 5.0f;
  float a = std::fabs(real)/factor;
  float b = std::fabs(imag)/factor;
  // Avoid flat ellipsis for edge cases (for real = 0, the case imag = 0 is excluded)
  if (real == 0.0f) {
    a = 1.0f/factor;
    th = imag < 0.0f ? -M_PI/2.0f : M_PI/2.0f;
  }
  std::complex<float> parameters(a,b);
  drawCurve(ctx, rect, 0.0f, 1.0f, 0.01f,
      [](float t, void * model, void * context) {
      std::complex<float> parameters = *(std::complex<float> *)model;
        float th = *(float *)context;
        float a = parameters.real();
        float b = parameters.imag();
        return Poincare::Coordinate2D<float>(a*std::cos(t*th), b*std::sin(t*th));
    }, &parameters, &th, false, Palette::GreyDark, false);

  // Draw dashed segment to indicate real and imaginary
  drawSegment(ctx, rect, Axis::Vertical, real, 0.0f, imag, Palette::Red, 1, 3);
  drawSegment(ctx, rect, Axis::Horizontal, imag, 0.0f, real, Palette::Red, 1, 3);

  // Draw complex position on the plan
  drawDot(ctx, rect, real, imag, Palette::Red, true);

  // Draw labels
  // 're(z)' label
  drawLabel(ctx, rect, real, 0.0f, "re(z)", Palette::Red, CurveView::RelativePosition::None, imag >= 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After);
  // 'im(z)' label
  drawLabel(ctx, rect, 0.0f, imag, "im(θ)", Palette::Red, real >= 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After, CurveView::RelativePosition::None);
  // '|z|' label, the relative horizontal position of this label depends on the quadrant
  CurveView::RelativePosition verticalPosition = real*imag < 0.0f ? CurveView::RelativePosition::Before : CurveView::RelativePosition::After;
  if (real == 0.0f) {
    // Edge case: pure imaginary
    verticalPosition = CurveView::RelativePosition::None;
  }
  drawLabel(ctx, rect, real/2.0f, imag/2.0f, "|z|", Palette::Red, CurveView::RelativePosition::None, verticalPosition);
  // 'arg(z)' label, the absolute and relative horizontal/vertical positions of this label depends on the quadrant
  CurveView::RelativePosition horizontalPosition = real >= 0.0f ? CurveView::RelativePosition::After : CurveView::RelativePosition::None;
  verticalPosition = imag >= 0.0f ? CurveView::RelativePosition::After : CurveView::RelativePosition::Before;
  /* anglePositionRatio is the ratio of the angle where we position the label
   * For the right half plan, we position the label close to the abscissa axis
   * and for the left half plan, we position the label at the half angle. The
   * relative position is chosen accordingly. */
  float anglePositionRatio = real >= 0.0f ? 0.0f : 0.5f;
  drawLabel(ctx, rect, a*std::cos(anglePositionRatio*th), b*std::sin(anglePositionRatio*th), "arg(z)", Palette::Red, horizontalPosition, verticalPosition);
}

}